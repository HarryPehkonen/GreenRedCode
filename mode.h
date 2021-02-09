/*
 * IncFile1.h
 *
 * Created: 2021-01-16 10:28:24
 *  Author: harry
 */ 


#ifndef MODE_H_
#define MODE_H_

typedef enum MODE {
	MODE_NONE,
	MODE_GREEN,
	MODE_RED,
	MODE_RECORD_GREEN,
	MODE_RECORD_RED,
	MODE_OFF
} mode_t;

void mode_pause(uint8_t ticks);
void mode_to(mode_t new_mode);
uint8_t mode_is(mode_t mode);
void mode_toggle();

#endif /* MODE_H_ */