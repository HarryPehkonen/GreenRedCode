/*
 * sirc.c
 *
 * Created: 2021-01-17 09:15:16
 *  Author: harry
 */ 

#include "freq.h"
#include <avr/io.h>
#include <stddef.h> /* NULL */
#include <avr/cpufunc.h> /* _NOP() */
#include <string.h> /* memset */
#include "sirc.h"
#include "util.h"

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

#define SIRC_TOKEN_START_MIN  (US_TO_TICKS(2400 - 100))
#define SIRC_TOKEN_START_MAX  (US_TO_TICKS(2400 + 100))
#define SIRC_TOKEN_LONG_MIN   (US_TO_TICKS(1200 - 100))
#define SIRC_TOKEN_LONG_MAX   (US_TO_TICKS(1200 + 100))
#define SIRC_TOKEN_SHORT_MIN  (US_TO_TICKS(600 - 100))
#define SIRC_TOKEN_SHORT_MAX  (US_TO_TICKS(600 + 100))
#define SIRC_TOKEN_SEPARATOR_MIN  (US_TO_TICKS(2600))
#define SIRC_TOKEN_SEPARATOR_MAX  (US_TO_TICKS(45000))

token_specs_t token_specs[] = {
	{
		.min = SIRC_TOKEN_SHORT_MIN,
		.max = SIRC_TOKEN_SHORT_MAX,
		.token = SIRC_TOKEN_SHORT /* 62 - 87 */
	}, {
		.min = SIRC_TOKEN_LONG_MIN,
		.max = SIRC_TOKEN_LONG_MAX,
		.token = SIRC_TOKEN_LONG /* 137 - 162 */
	}, {
		.min = SIRC_TOKEN_START_MIN,
		.max = SIRC_TOKEN_START_MAX,
		.token = SIRC_TOKEN_START /* 287 - 312 */
	}, {
		.min = SIRC_TOKEN_SEPARATOR_MIN,
		.max = SIRC_TOKEN_SEPARATOR_MAX,
		.token = SIRC_TOKEN_SEPARATOR /* 325 - 5625*/
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

typedef enum state_t {
	SIRC_STATE_NONE = 0,
	SIRC_STATE_NEED_START_SHORT = 1,
	SIRC_STATE_HAVE_START = 2,
	SIRC_STATE_NEED_SHORT = 3
} state_t;

/* _message and _mask must be the same type */
static message_t _message = 0;
static message_t _mask = 0;

static state_t _state = SIRC_STATE_NONE;
void (*on_message)(message_t);

void
sirc_set_on_message(void (*fn)(message_t)) {
	on_message = fn;
}

void
sirc_edge(uint16_t ticks) {
	static uint16_t prev_ticks = 0;
	uint16_t delta_ticks = (ticks > prev_ticks) ? (ticks - prev_ticks) : ((0xFFFF - prev_ticks) + ticks);
	token_t token = _sirc_tokenize(delta_ticks);
	if (_state == SIRC_STATE_NONE) {
		if (token == SIRC_TOKEN_START) {
			_state = SIRC_STATE_NEED_START_SHORT;
		}
	} else if (_state == SIRC_STATE_NEED_START_SHORT) {
		if (token == SIRC_TOKEN_SHORT) {
			_state = SIRC_STATE_HAVE_START;
			_message = 0;
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
			_message |= _mask;
			_mask <<= 1;
		} else {
			
			/* bombed */
			_state = SIRC_STATE_NONE;
		}
		
	} else if (_state == SIRC_STATE_NEED_SHORT) {
		if (token == SIRC_TOKEN_SHORT) {
			_state = SIRC_STATE_HAVE_START;
		} else if (token == SIRC_TOKEN_SEPARATOR) {
			
			/* callback */
			_state = SIRC_STATE_NONE;
			on_message(_message);
		} else {

			/* bombed */
			_state = SIRC_STATE_NONE;
		}
	}

	prev_ticks = ticks;
}

static uint8_t _tokenizer_worked = 0;
void _tokenizer_works(message_t message) {
	if (message == 0x96) {
		_tokenizer_worked = 0xff;
	}
}

void _poweroff_works(message_t message) {
	if (message == 0x95) {
		_tokenizer_worked = 0xff;
	}
}

uint16_t loop;
void
sirc_test() {

	/* test _sirc_tokenize */
	if(_sirc_tokenize(SIRC_TOKEN_START_MIN - 1) != SIRC_TOKEN_UNKNOWN) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_START_MIN) != SIRC_TOKEN_START) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_START_MAX) != SIRC_TOKEN_START) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_START_MAX + 1) != SIRC_TOKEN_UNKNOWN) while(1);

	if(_sirc_tokenize(SIRC_TOKEN_LONG_MIN - 1) != SIRC_TOKEN_UNKNOWN) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_LONG_MIN) != SIRC_TOKEN_LONG) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_LONG_MAX) != SIRC_TOKEN_LONG) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_LONG_MAX + 1) != SIRC_TOKEN_UNKNOWN) while(1);

	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MIN - 1) != SIRC_TOKEN_UNKNOWN) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MIN) != SIRC_TOKEN_SHORT) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MAX) != SIRC_TOKEN_SHORT) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SHORT_MAX + 1) != SIRC_TOKEN_UNKNOWN) while(1);

	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MIN - 1) != SIRC_TOKEN_UNKNOWN) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MIN) != SIRC_TOKEN_SEPARATOR) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MAX) != SIRC_TOKEN_SEPARATOR) while(1);
	if(_sirc_tokenize(SIRC_TOKEN_SEPARATOR_MAX + 1) != SIRC_TOKEN_UNKNOWN) while(1);

	/* test sirc_edge */
	sirc_set_on_message(_tokenizer_works);

	uint16_t ticks = 0;

	sirc_edge(ticks);
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

	/* stop burst */
	ticks += SIRC_TOKEN_SHORT_MIN;
	sirc_edge(ticks);

	ticks += SIRC_TOKEN_SEPARATOR_MIN;
	sirc_edge(ticks);

	if (_tokenizer_worked == 0) while(1);

	/* XXX:  fragile test -- requires 1/64 prescaler and 8MHz */
	sirc_set_on_message(_poweroff_works);
	_tokenizer_worked = 0;
	ticks = 0;
	sirc_edge(ticks);
	uint16_t poweroff[] = {
		307,
		70,
		156,
		70,
		80,
		70,
		156,
		70,
		81,
		69,
		156,
		70,
		81,
		69,
		81,
		70,
		156,
		70,
		80,
		70,
		81,
		69,
		87,
		70,
		81,
		3237
	};
	for(uint8_t n = 0; n < (sizeof(poweroff) / sizeof(poweroff[0])); n += 1) {
		ticks += poweroff[n];
		sirc_edge(ticks);
	}
	if (_tokenizer_worked == 0) while(1);

	/* remove test callback */
	sirc_set_on_message(NULL);

}
