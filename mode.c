/*
 * mode.c
 *
 * Created: 2021-01-16 10:28:56
 *  Author: harry
 */

#include <avr/io.h>
#include "mode.h"
#include "pins.h"

mode_t _mode = MODE_GREEN;
uint8_t _pause_ticks = 0;

void
mode_pause(uint8_t ticks) {
	_pause_ticks = ticks;
}

void
mode_to(mode_t new_mode) {

	_mode = new_mode;

	pins_green1_off();
	pins_green2_off();
	pins_green_back_off();
	pins_red1_off();
	pins_red2_off();
	pins_red_back_off();

	if (_mode == MODE_OFF) {
		pins_irvcc_off();
	} else {
		pins_irvcc_on();
	}

	/* some modes require pins to be out of phase */
	if (new_mode == MODE_GREEN) {
		pins_green1_on();
	} else if (new_mode == MODE_RED) {
		pins_red1_on();
	}
}

uint8_t
mode_is(mode_t mode) {
	return _mode == mode;
}

void
mode_toggle() {
	if (_mode == MODE_GREEN) {
		mode_to(MODE_RED);
	} else if (_mode == MODE_RED) {
		mode_to(MODE_GREEN);
	}
}
