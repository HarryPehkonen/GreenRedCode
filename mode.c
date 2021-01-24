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
mode_tick() {
	static uint8_t prescale2 = 0;
	static uint8_t prescale3 = 0;

	if (_pause_ticks > 0) {
		_pause_ticks -= 1;
		return;
	}
	
	if (prescale2 == 0) {
		prescale2 = 0x06;

		if (_mode == MODE_GREEN) {
			pins_green1_toggle();
			pins_green2_toggle();
			pins_green_back_toggle();
		} else if (_mode == MODE_RED) {
			pins_red1_toggle();
			pins_red2_toggle();
			pins_red_back_toggle();
		}
	}

	if (prescale3 == 0) {
		prescale3 = 0x03;

		if (_mode == MODE_RECORD_GREEN) {
			pins_green_back_toggle();
		} else if (_mode == MODE_RECORD_RED) {
			pins_red_back_toggle();
		}
	}

	prescale2 -= 1;
	prescale3 -= 1;
}

void
mode_pause(uint8_t ticks) {
	_pause_ticks = ticks;
}

void
mode_to(mode_t new_mode) {

	/* ignore while paused */
	if (_pause_ticks) {
		return;
	}

	if (new_mode == MODE_RED) {
		pins_green1_off();
		pins_green2_off();
		pins_red2_off();
		pins_green_back_off();
		pins_red1_on();
	} else if (new_mode == MODE_GREEN) {
		pins_red1_off();
		pins_red2_off();
		pins_green2_off();
		pins_red_back_off();
		pins_green1_on();
	} else if (new_mode == MODE_RECORD_GREEN) {
		pins_red1_off();
		pins_red2_off();
		pins_green1_off();
		pins_green2_off();
		pins_red_back_off();
		pins_green_back_on();
	} else if (new_mode == MODE_RECORD_RED) {
		pins_red1_off();
		pins_red2_off();
		pins_green1_off();
		pins_green2_off();
		pins_red_back_on();
		pins_green_back_off();
	}
	_mode = new_mode;
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
