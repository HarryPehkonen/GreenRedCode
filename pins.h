/*
 * pins.h
 *
 * Created: 2021-01-15 09:36:21
 *  Author: harry
 */ 


#ifndef PINS_H_
#define PINS_H_

void pins_init();
void pins_off();
void pins_red_back_on();
void pins_red_back_off();
void pins_red_back_toggle();
void pins_red1_on();
void pins_red1_off();
void pins_red1_toggle();
void pins_red2_on();
void pins_red2_off();
void pins_red2_toggle();
void pins_green_back_on();
void pins_green_back_off();
void pins_green_back_toggle();
void pins_green1_on();
void pins_green1_off();
void pins_green1_toggle();
void pins_green2_on();
void pins_green2_off();
void pins_green2_toggle();
void pins_irvcc_on();
void pins_irvcc_off();
void pins_irvcc_toggle();
uint8_t pins_button_is_pressed();

#endif /* PINS_H_ */