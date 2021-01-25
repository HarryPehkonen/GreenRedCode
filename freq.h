/*
 * freq.h
 *
 * Created: 2021-01-15 09:34:54
 *  Author: harry
 */ 


#ifndef FREQ_H_
#define FREQ_H_

#define F_CPU 8000000
#define PRESCALE 64
#define GREEN_EEPROM_ADDR 0
#define RED_EEPROM_ADDR 4

#define US_TO_TICKS(us) ((us) * (F_CPU / 1000000UL) / PRESCALE)

#endif /* FREQ_H_ */