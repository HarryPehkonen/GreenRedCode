/*
 * pins.c
 *
 * Created: 2021-01-15 09:37:27
 *  Author: harry
 */ 

#include <avr/io.h>

#define DDR_RED_BACK DDRD
#define PORT_RED_BACK PORTD
#define PIN_RED_BACK PIND
#define BIT_RED_BACK 2

#define DDR_RED1 DDRD
#define PORT_RED1 PORTD
#define PIN_RED1 PIND
#define BIT_RED1 0

#define DDR_RED2 DDRD
#define PORT_RED2 PORTD
#define PIN_RED2 PIND
#define BIT_RED2 1

#define DDR_GREEN_BACK DDRD
#define PORT_GREEN_BACK PORTD
#define PIN_GREEN_BACK PIND
#define BIT_GREEN_BACK 5

#define DDR_GREEN1 DDRD
#define PORT_GREEN1 PORTD
#define PIN_GREEN1 PIND
#define BIT_GREEN1 3

#define DDR_GREEN2 DDRD
#define PORT_GREEN2 PORTD
#define PIN_GREEN2 PIND
#define BIT_GREEN2 4

#define DDR_BTN DDRD
#define PORT_BTN PORTD
#define PIN_BTN PIND
#define BIT_BTN 6

void
pins_init() {

	/* output */
	DDR_RED_BACK |= (1<<BIT_RED_BACK);
	DDR_RED1 |= (1<<BIT_RED1);
	DDR_RED2 |= (1<<BIT_RED2);

	DDR_GREEN_BACK |= (1<<BIT_GREEN_BACK);
	DDR_GREEN1 |= (1<<BIT_GREEN1);
	DDR_GREEN2 |= (1<<BIT_GREEN2);

	/* input */
	DDR_BTN |= (0<<BIT_BTN); /* no effect */
	PORT_BTN |= (1<<BIT_BTN); /* pull-up */

	/* interrupt on pin change on push-button */
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<BIT_BTN);
}

void
pins_red_back_on() {
	PORT_RED_BACK |= (1<<BIT_RED_BACK);
}

void
pins_red_back_off() {
	PORT_RED_BACK &= ~(1<<BIT_RED_BACK);
}

void
pins_red_back_toggle() {
	PIN_RED_BACK = (1<<BIT_RED_BACK);
}

void
pins_red1_on() {
	PORT_RED1 |= (1<<BIT_RED1);
}

void
pins_red1_off() {
	PORT_RED1 &= ~(1<<BIT_RED1);
}

void
pins_red1_toggle() {
	PIN_RED1 = (1<<BIT_RED1);
}

void
pins_red2_on() {
	PORT_RED2 |= (1<<BIT_RED2);
}

void
pins_red2_off() {
	PORT_RED2 &= ~(1<<BIT_RED2);
}

void
pins_red2_toggle() {
	PIN_RED2 = (1<<BIT_RED2);
}

void
pins_green_back_on() {
	PORT_GREEN_BACK |= (1<<BIT_GREEN_BACK);
}

void
pins_green_back_off() {
	PORT_GREEN_BACK &= ~(1<<BIT_GREEN_BACK);
}

void
pins_green_back_toggle() {
	PIN_GREEN_BACK = (1<<BIT_GREEN_BACK);
}

void
pins_green1_on() {
	PORT_GREEN1 |= (1<<BIT_GREEN1);
}

void
pins_green1_off() {
	PORT_GREEN1 &= ~(1<<BIT_GREEN1);
}

void
pins_green1_toggle() {
	PIN_GREEN1 = (1<<BIT_GREEN1);
}

void
pins_green2_on() {
	PORT_GREEN2 |= (1<<BIT_GREEN2);
}

void
pins_green2_off() {
	PORT_GREEN2 &= ~(1<<BIT_GREEN2);
}

void
pins_green2_toggle() {
	PIN_GREEN2 = (1<<BIT_GREEN2);
}

uint8_t
pins_button_is_pressed() {
	return PIN_BTN | BIT_BTN;
}
