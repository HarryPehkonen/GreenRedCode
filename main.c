/*
 * GreenRed.c
 *
 * Created: 2021-01-15 09:34:35
 * Author : harry
 */ 

#include "freq.h"

#include <stddef.h> /* NULL */
#include <avr/cpufunc.h> /* _NOP() */
#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "counter0.h"
#include "counter1.h"
#include "pins.h"
#include "mode.h"
#include "sirc.h"
#include "buffer.h"

buffer_t *msgbuf;

void
on_sirc_message(message_t msg) {
	buffer_push(msgbuf, msg);
}

void
write_green_message(message_t msg) {
	eeprom_write_dword((uint32_t *)GREEN_EEPROM_ADDR, msg);
	do {} while (!eeprom_is_ready());
}

uint32_t
read_green_message() {
	return eeprom_read_dword((uint32_t *)GREEN_EEPROM_ADDR);
}

void
write_red_message(message_t msg) {
	eeprom_write_dword((uint32_t *)RED_EEPROM_ADDR, msg);
	do {} while (!eeprom_is_ready());
}

uint32_t
read_red_message() {
	return eeprom_read_dword((uint32_t *)RED_EEPROM_ADDR);
}

int main(void)
{
	sirc_test();
	buffer_test();

	msgbuf = buffer_create();

	pins_init();
	counter0_init();
	counter0_set_ocra(0xff);

	counter1_init();
	
	mode_to(MODE_GREEN);

	sirc_set_on_message(on_sirc_message);
	sei();
	
	while(1) {
		if (!buffer_is_empty(msgbuf)) {
			if (mode_is(MODE_GREEN) || mode_is(MODE_RED)) {
				uint32_t msg = buffer_pop(msgbuf);
				if (msg == read_green_message()) {
					mode_to(MODE_GREEN);
				} else if (msg == read_red_message()) {
					mode_to(MODE_RED);
				}
			} else if (mode_is(MODE_RECORD_GREEN)) {
				message_t msg = buffer_pop(msgbuf);
				write_green_message(msg);
				mode_to(MODE_RECORD_RED);
			} else if (mode_is(MODE_RECORD_RED)) {

				/* make sure we don't just read a repeated green code */
				message_t green = read_green_message();
				while (1) {
					/* wait until there is something in the buffer */
					while (buffer_is_empty(msgbuf));

					uint32_t msg = buffer_pop(msgbuf);
					if (msg == green) {
						continue;
					}
					write_red_message(msg);
					mode_to(MODE_GREEN);

					/*
					   ignore any events in order to flush out
					   any remaining IR codes
					 */
					mode_pause(30);
					break;
				}
			}
		}
	}
}

/* heart-beat */
ISR(TIMER0_COMPA_vect) {
	mode_tick();
}

/* for IR */
ISR(TIMER1_CAPT_vect) {

	/* toggle edge select */
	TCCR1B ^= (1<<ICES1);
	
	uint16_t ticks = ICR1;
	sirc_edge(ticks);
}

/* button press */
ISR(PCINT2_vect) {
	mode_to(MODE_RECORD_GREEN);
}
