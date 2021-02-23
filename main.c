/*
 * GreenRed.c
 *
 * Created: 2021-01-15 09:34:35
 * Author : harry
 */

#include "freq.h"

#include <stddef.h> /* NULL */
#include <avr/cpufunc.h> /* _NOP() */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h> /* debounce */
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "counter0.h"
#include "counter1.h"
#include "pins.h"
#include "mode.h"
#include "sirc.h"
#include "queue.h"

queue_t *code_queue;
volatile uint8_t  ticks_have_occurred = 0;

void
wdt_init() {
	wdt_enable(WDTO_250MS);
}

void
runtime_power() {
	PRR = PRR
			| (1<<PRTWI)
			/* | (1<<PRTIM0) */
			/* | (1<<PRTIM1) */
			| (1<<PRTIM2)
			| (1<<PRSPI)
			| (1<<PRUSART0)
			| (1<<PRADC);
}

void
off_power() {

	wdt_disable();
	pins_off();

	/* sleep mode requires pin-change interrupt on button */
	/* sleep mode:  power-down */
	SMCR |= (2<<SM0);
	sleep_enable();
	sleep_bod_disable();
	sleep_cpu();
	sleep_disable();

	pins_init();
	wdt_init();
}

void
on_sirc_code(sirc_code_t code) {
	if (!SIRC_GET_IS_REPEAT(code)) {
		queue_enqueue(code_queue, code);
	}
}

void
write_green_code(sirc_code_t code) {
	eeprom_write_dword((uint32_t *)GREEN_EEPROM_ADDR, code);
	do {} while (!eeprom_is_ready());
}

uint32_t
read_green_code() {
	return eeprom_read_dword((uint32_t *)GREEN_EEPROM_ADDR);
}

void
write_red_code(sirc_code_t code) {
	eeprom_write_dword((uint32_t *)RED_EEPROM_ADDR, code);
	do {} while (!eeprom_is_ready());
}

uint32_t
read_red_code() {
	return eeprom_read_dword((uint32_t *)RED_EEPROM_ADDR);
}

void
on_button_short_press() {

	if (mode_is(MODE_OFF)) {
		mode_to(MODE_GREEN);
	} else {
		mode_to(MODE_OFF);
	}
}

void
on_button_long_press() {
	if (mode_is(MODE_OFF)) {
		return;
	}

	if (mode_is(MODE_RECORD_GREEN)) {

		/* cancel recording of green code */
		mode_to(MODE_GREEN);
	} else if (mode_is(MODE_RECORD_RED)) {

		/* cancel recording of red code */
		mode_to(MODE_RED);
	} else  if (mode_is(MODE_GREEN)) {

		/*
		 * start recording green code.  once it's recorded,
		 * automatically switch to recording red code.
		 */
		mode_to(MODE_RECORD_GREEN);
	} else if (mode_is(MODE_RED)) {

		/*
		 * go straight to recording red code.  once it's recorded, get
		 * out of recording mode.
		 */
		 mode_to(MODE_RECORD_RED);
	}
}

void
run_sync() {
	static uint8_t prescale2 = 0;
	static uint8_t prescale3 = 0;
	static uint8_t button_was_pressed = 0;

	if (pins_button_is_pressed()) {
		button_was_pressed += 1;
	} else {

		/* button is no longer pressed */
		if ((button_was_pressed >= 2) && (button_was_pressed < 50)) {
			on_button_short_press();
		} else if (button_was_pressed >= 51) {
			on_button_long_press();
		}
		button_was_pressed = 0;
	}

	if (prescale2 == 0) {
		prescale2 = 0x06;

		if (mode_is(MODE_GREEN)) {
			pins_green1_toggle();
			pins_green2_toggle();
			pins_green_back_toggle();
		} else if (mode_is(MODE_RED)) {
			pins_red1_toggle();
			pins_red2_toggle();
			pins_red_back_toggle();
		}
	}

	if (prescale3 == 0) {
		prescale3 = 0x03;

		if (mode_is(MODE_RECORD_GREEN)) {
			pins_green_back_toggle();
		} else if (mode_is(MODE_RECORD_RED)) {
			pins_red_back_toggle();
		}
	}

	prescale2 -= 1;
	prescale3 -= 1;
}

void
run_async() {

	/* deal with any IR codes that were received */
	if (!queue_is_empty(code_queue)) {
		if (mode_is(MODE_GREEN) || mode_is(MODE_RED)) {
			uint32_t code = queue_dequeue(code_queue);
			if (code == read_green_code()) {
				mode_to(MODE_GREEN);
			} else if (code == read_red_code()) {
				mode_to(MODE_RED);
			}
		} else if (mode_is(MODE_RECORD_GREEN)) {
			sirc_code_t code = queue_dequeue(code_queue);
			write_green_code(code);
			mode_to(MODE_RECORD_RED);
		} else if (mode_is(MODE_RECORD_RED)) {

			uint32_t new_code = queue_dequeue(code_queue);

			/* make sure we don't just read a repeated green code */
			sirc_code_t green = read_green_code();
			if (new_code != green) {
				write_red_code(new_code);
				mode_to(MODE_GREEN);
			}
			/* else remain in MODE_RECORD_RED */
		}
	}

	if (mode_is(MODE_OFF)) {
		off_power();
	}
}

int main(void)
{
#ifdef DEBUG
	sirc_test();
	queue_test();
#endif

	code_queue = queue_create();

	pins_init();
	counter0_init();
	counter0_set_ocra(0xff);
	counter1_init();
	runtime_power();
	mode_to(MODE_GREEN);
	sirc_set_on_code(on_sirc_code);
	wdt_init();
	sei();

	while(1) {
		run_async();
		if (ticks_have_occurred) {
			ticks_have_occurred -= 1;
			run_sync();
			wdt_reset();
		}
	}
}

/* heart-beat */
ISR(TIMER0_COMPA_vect) {
	ticks_have_occurred += 1;
}

/* for IR */
ISR(TIMER1_CAPT_vect) {

	/* toggle edge select */
	TCCR1B ^= (1<<ICES1);

	uint16_t ticks = ICR1;
	sirc_edge(ticks);
}

/* button press */
ISR(PCINT2_vect) {
	if (mode_is(MODE_OFF)) {
		if (pins_button_is_pressed()) {

			/* wait for button to be released */
			while(pins_button_is_pressed());

			mode_to(MODE_GREEN);
		}
	}
	return;
}
