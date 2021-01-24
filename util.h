/*
 * util.h
 *
 * Created: 2021-01-17 16:22:42
 *  Author: harry
 */ 


#ifndef UTIL_H_
#define UTIL_H_

uint16_t us_to_ticks(uint32_t us, uint16_t prescale, uint32_t hz);
uint16_t ticks_to_us(uint32_t ticks, uint16_t prescale, uint32_t hz);

#define US_TO_TICKS(us) ((us) * (F_CPU / 1000000UL) / PRESCALE)
void util_test();

#endif /* UTIL_H_ */