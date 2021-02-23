/*
 * counter0.c
 *
 * Created: 2021-01-16 10:05:33
 *  Author: harry
 */ 

#include <avr/io.h>
#include "freq.h"
#include "counter0.h"

void
counter0_init() {
	/* CTC mode */
	TCCR0A |= (2<<WGM00);

	/* prescaler */
	/* 5:  1/1024 */
	TCCR0B |= (5<<CS00);

	/* enable interrupt */
	TIMSK0 |= (1<<OCIE0A);
}

void
counter0_set_ocra(uint8_t count) {
	OCR0A = count;
}
