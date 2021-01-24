/*
 * util.c
 *
 * Created: 2021-01-17 16:22:30
 *  Author: harry
 */ 

#include "freq.h"
#include <avr/io.h>
#include "util.h"

uint16_t
us_to_ticks(uint32_t us, uint16_t prescale, uint32_t hz) {
	return us * (hz / 1000000) / prescale;
}

uint16_t
ticks_to_us(uint32_t ticks, uint16_t prescale, uint32_t hz) {
	return ticks * prescale / (hz / 1000000);
}

void
util_test() {
    volatile uint32_t expected;
    volatile uint32_t actual;

    /* ticks_to_us */
    expected = 0;
    actual = ticks_to_us(0, 64, 8000000);
    if(expected != actual) while(1);

    expected = 800; //100 * 64 / 8000000;
    actual = ticks_to_us(100, 64, 8000000);
    if(expected != actual) while(1);

    expected = 1256;
    actual = ticks_to_us(157, 64, 8000000);
    if(expected != actual) while(1);

    expected = 25000;
    actual = ticks_to_us(3125, 64, 8000000);
    if(expected != actual) while(1);

    /* us_to_ticks */
    expected = 157;
    actual = us_to_ticks(1256, 64, 8000000);
    if(expected != actual) while(1);

    expected = 3125;
    actual = us_to_ticks(25000, 64, 8000000);
    if(expected != actual) while(1);
}
