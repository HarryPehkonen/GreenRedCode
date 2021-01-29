/*
 * sirc.c
 *
 * Created: 2021-01-17 09:15:16
 *  Author: harry
 */ 

#include <avr/io.h>
#include "freq.h"
#include <stddef.h> /* NULL */
#include "sirc.h"

typedef enum token_t {
	SIRC_TOKEN_UNKNOWN,
	SIRC_TOKEN_START,
	SIRC_TOKEN_LONG,
	SIRC_TOKEN_SHORT,
	SIRC_TOKEN_SEPARATOR
} token_t;

typedef struct token_specs_t {
	uint16_t min;
	uint16_t max;
	token_t token;
} token_specs_t;

/*
 * WARNING:  because of different granularity between microseconds and ticks,
 * there is overlap between some microsecond values.  for example, 600us, at 8
 * MHz, and with prescale of 64, takes 75 ticks.  76 ticks takes 608us.
 * therefore 600us is exactly equal to 607us.  make sure there is enough
 * difference between the MAX of one range and the MIN of the next range (i.e.
 * TOKEN_START_MAX and TOKEN_SEPARATOR_MIN).
 */
#define SIRC_TOKEN_SHORT_MIN  (US_TO_TICKS(600 - 100))
#define SIRC_TOKEN_SHORT_MAX  (US_TO_TICKS(600 + 120))
#define SIRC_TOKEN_LONG_MIN   (US_TO_TICKS(1200 - 120))
#define SIRC_TOKEN_LONG_MAX   (US_TO_TICKS(1200 + 120))
#define SIRC_TOKEN_START_MIN  (US_TO_TICKS(2400 - 240))
#define SIRC_TOKEN_START_MAX  (US_TO_TICKS(2400 + 240))
#define SIRC_TOKEN_SEPARATOR_MIN  (US_TO_TICKS(2700))
#define SIRC_TOKEN_SEPARATOR_MAX  (US_TO_TICKS(45000))

/* table for analyzing a pulse based on its length */
token_specs_t token_specs[] = {
	{
		.min = SIRC_TOKEN_SHORT_MIN,
		.max = SIRC_TOKEN_SHORT_MAX,
		.token = SIRC_TOKEN_SHORT
	}, {
		.min = SIRC_TOKEN_LONG_MIN,
		.max = SIRC_TOKEN_LONG_MAX,
		.token = SIRC_TOKEN_LONG
	}, {
		.min = SIRC_TOKEN_START_MIN,
		.max = SIRC_TOKEN_START_MAX,
		.token = SIRC_TOKEN_START
	}, {
		.min = SIRC_TOKEN_SEPARATOR_MIN,
		.max = SIRC_TOKEN_SEPARATOR_MAX,
		.token = SIRC_TOKEN_SEPARATOR
	}
};

static token_t
_sirc_tokenize(uint16_t delta_ticks) {
	token_t result = SIRC_TOKEN_UNKNOWN;
	for(uint8_t i = 0; i < (sizeof(token_specs) / sizeof(token_specs[0])); i += 1) {
		if (delta_ticks >= token_specs[i].min && delta_ticks <= token_specs[i].max) {
			result = token_specs[i].token;
			break;
		}
	}

	return result;
}

/* callback */
void (*on_code)(sirc_code_t);

void
sirc_set_on_code(void (*fn)(sirc_code_t)) {
	on_code = fn;
}

/* must be the same type */
sirc_code_t _code = 0;
sirc_code_t _mask = 0;
sirc_code_t _prev_code = 0;
sirc_code_t _code_length = 0;

/* state machine */
typedef enum state_t {
	SIRC_STATE_NONE = 0,
	SIRC_STATE_NEED_START_SHORT = 1,
	SIRC_STATE_HAVE_START = 2,
	SIRC_STATE_NEED_SHORT = 3
} state_t;

static state_t _state = SIRC_STATE_NONE;

/*
 * whenever a pulse starts or ends, advise sirc_edge how long the pulse or
 * space was (in ticks).
 */
void
sirc_edge(uint16_t ticks) {
	static uint16_t prev_ticks = 0;
	uint16_t delta_ticks = (ticks > prev_ticks) ? (ticks - prev_ticks) : ((0xFFFF - prev_ticks) + ticks);
	token_t token = _sirc_tokenize(delta_ticks);
	if (_state == SIRC_STATE_NONE) {
		if (token == SIRC_TOKEN_START) {
			_state = SIRC_STATE_NEED_START_SHORT;
		} else if (token == SIRC_TOKEN_UNKNOWN) {

			/* XXX:  get rid of this? */
			/*
			 * suspect we received a very long pause in between pulses. that
			 * means the next code should not be considered a repeat, even if
			 * it's the same one as before.
			 */
			 //_prev_code = 0;
		}
	} else if (_state == SIRC_STATE_NEED_START_SHORT) {
		if (token == SIRC_TOKEN_SHORT) {
			_state = SIRC_STATE_HAVE_START;
			_code = 0;
			_code_length = 0;
			_mask = 1;
		} else {

			/* bombed */
			_state = SIRC_STATE_NONE;
		}
	} else if (_state == SIRC_STATE_HAVE_START) {
		if (token == SIRC_TOKEN_SHORT) {
			_state = SIRC_STATE_NEED_SHORT;
			_mask <<= 1;
		} else if (token == SIRC_TOKEN_LONG) {
			_state = SIRC_STATE_NEED_SHORT;
			_code |= _mask;
			_mask <<= 1;
		} else {

			/* bombed */
			_state = SIRC_STATE_NONE;
		}

	} else if (_state == SIRC_STATE_NEED_SHORT) {
		if (token == SIRC_TOKEN_SHORT) {
			_state = SIRC_STATE_HAVE_START;
			_code_length += 1;
		} else if (token == SIRC_TOKEN_SEPARATOR) {

			uint8_t is_repeat = (_prev_code == _code);

			/* remember code *before* adding length to it */
			_prev_code = _code;

			/* previous pulse matters */
			_code_length += 1;

			/* add code length to end */
			/* see .h for meaning */
			_code_length <<= SIRC_LENGTH_SHIFT_AMOUNT;
			_code |= _code_length;
			if (is_repeat) {
				_code |= SIRC_IS_REPEAT_MASK;
			}

			_state = SIRC_STATE_NONE;

			/* callback */
			if (on_code != NULL) {
				on_code(_code);
			}
		} else {

			/*
			 * suspect we just had a long pause (longer than a separator). that
			 * means the next code should not be considered a repeat.  make
			 * _prev_code look like something that can't possibly match the
			 * next code.
			 */
			_prev_code = 0;
			_state = SIRC_STATE_NONE;
		}
	}

	prev_ticks = ticks;
}

/* only tests follow */
#ifdef DEBUG
static uint8_t _tokenizer_worked = 0;
void
_tokenizer_works(sirc_code_t code) {
	if ((SIRC_GET_CODE(code) == 0x96) && (SIRC_GET_LENGTH(code) == 12)) {
		_tokenizer_worked = 0xff;
	}
}

void
_poweroff_works(sirc_code_t code) {
	if ((SIRC_GET_CODE(code) == 0x95) && (SIRC_GET_LENGTH(code) == 12)) {
		_tokenizer_worked = 0xff;
	}
}

void
sirc_test() {

	/* test _sirc_tokenize */
	if(_sirc_tokenize(SIRC_TOKEN_START_MIN - 1) == SIRC_TOKEN_START) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_START_MIN) != SIRC_TOKEN_START) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_START_MAX) != SIRC_TOKEN_START) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_START_MAX + 1) == SIRC_TOKEN_START) while(1);

	if(_sirc_tokenize(SIRC_TOKEN_LONG_MIN - 1) == SIRC_TOKEN_LONG) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_LONG_MIN) != SIRC_TOKEN_LONG) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_LONG_MAX) != SIRC_TOKEN_LONG) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_LONG_MAX + 1) == SIRC_TOKEN_LONG) while(1);

	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MIN - 1) == SIRC_TOKEN_SHORT) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MIN) != SIRC_TOKEN_SHORT) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MAX) != SIRC_TOKEN_SHORT) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MAX + 1) == SIRC_TOKEN_SHORT) while(1);

	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MIN - 1) == SIRC_TOKEN_SEPARATOR) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MIN) != SIRC_TOKEN_SEPARATOR) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MAX) != SIRC_TOKEN_SEPARATOR) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MAX + 1) == SIRC_TOKEN_SEPARATOR) while(1);

	/* test sirc_edge */
	sirc_set_on_code(_tokenizer_works);

	uint16_t ticks = 0;

	//sirc_edge(ticks);
	ticks += SIRC_TOKEN_START_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);


	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_LONG_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_LONG_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);


	ticks += SIRC_TOKEN_LONG_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_LONG_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);


	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	/* stop burst */
	ticks += SIRC_TOKEN_SEPARATOR_MIN;
	sirc_edge(ticks);

	if (_tokenizer_worked == 0) while(1);

	sirc_set_on_code(_poweroff_works);
	_tokenizer_worked = 0;
	//ticks = 0;
	//sirc_edge(ticks);
	uint16_t poweroff[] = {
		US_TO_TICKS(2456),
		US_TO_TICKS(560),
		US_TO_TICKS(1248),
		US_TO_TICKS(560),
		US_TO_TICKS(640),
		US_TO_TICKS(560),
		US_TO_TICKS(1248),
		US_TO_TICKS(560),
		US_TO_TICKS(648),
		US_TO_TICKS(552),
		US_TO_TICKS(1248),
		US_TO_TICKS(560),
		US_TO_TICKS(648),
		US_TO_TICKS(552),
		US_TO_TICKS(648),
		US_TO_TICKS(560),
		US_TO_TICKS(1248),
		US_TO_TICKS(560),
		US_TO_TICKS(640),
		US_TO_TICKS(560),
		US_TO_TICKS(648),
		US_TO_TICKS(552),
		US_TO_TICKS(696),
		US_TO_TICKS(560),
		US_TO_TICKS(648),
		US_TO_TICKS(25896)
	};
	for(uint8_t n = 0; n < (sizeof(poweroff) / sizeof(poweroff[0])); n += 1) {
		ticks += poweroff[n];
		sirc_edge(ticks);
	}
	if (_tokenizer_worked == 0) while(1);

	/* remove test callback */
	sirc_set_on_code(NULL);

}
#endif /* DEBUG */
