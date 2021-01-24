/*
 * counter1.c
 *
 * Created: 2021-01-16 14:24:16
 *  Author: harry
 */ 

#include <avr/io.h>
#include "counter1.h"

void
counter1_init() {

	/* input capture */
	TIMSK1 |= (1<<ICIE1);
	
	/* normal mode; pins normal mode */
	
	/* prescaler */
	TCCR1B |= (3<<CS10);
}
