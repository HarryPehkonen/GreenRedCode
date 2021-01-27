/*
 * GreenRed.c
 *
 * Created: 2021-01-15 09:34:35
 * Author : harry
 */ 


#include <stddef.h> /* NULL */
#include <avr/cpufunc.h> /* _NOP() */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "freq.h"
#include "counter0.h"
#include "counter1.h"
#include "pins.h"
#include "mode.h"
#include "sirc.h"
#include "queue.h"

queue_t *code_queue;

void
on_sirc_code(sirc_code_t code) {
	if (!SIRC_GET_IS_REPEAT(code)) {
		queue_enqueue(code_queue, code);
	}
}

void
write_green_code(sirc_code_t code) {
	eeprom_write_dword((uint32_t *)GREEN_EEPROM_ADDR, code);
	do {} while (!eeprom_is_ready());
}

uint32_t
read_green_code() {
	return eeprom_read_dword((uint32_t *)GREEN_EEPROM_ADDR);
}

void
write_red_code(sirc_code_t code) {
	eeprom_write_dword((uint32_t *)RED_EEPROM_ADDR, code);
	do {} while (!eeprom_is_ready());
}

uint32_t
read_red_code() {
	return eeprom_read_dword((uint32_t *)RED_EEPROM_ADDR);
}

int main(void)
{
#ifdef DEBUG
	sirc_test();
	queue_test();
#endif

	code_queue = queue_create();

	pins_init();
	counter0_init();
	counter0_set_ocra(0xff);

	counter1_init();

	mode_to(MODE_GREEN);

	sirc_set_on_code(on_sirc_code);
	sei();

	while(1) {
		if (!queue_is_empty(code_queue)) {
			if (mode_is(MODE_GREEN) || mode_is(MODE_RED)) {
				uint32_t code = queue_dequeue(code_queue);
				if (code == read_green_code()) {
					mode_to(MODE_GREEN);
				} else if (code == read_red_code()) {
					mode_to(MODE_RED);
				}
			} else if (mode_is(MODE_RECORD_GREEN)) {
				sirc_code_t code = queue_dequeue(code_queue);
				write_green_code(code);
				mode_to(MODE_RECORD_RED);
			} else if (mode_is(MODE_RECORD_RED)) {

				/* make sure we don't just read a repeated green code */
				sirc_code_t green = read_green_code();
				while (1) {

					/* wait until there is something in the queue */
					while (queue_is_empty(code_queue));

					uint32_t code = queue_dequeue(code_queue);
					if (code == green) {
						continue;
					}
					write_red_code(code);
					mode_to(MODE_GREEN);

					/*
					   ignore any events in order to flush out
					   any remaining IR codes
					 */
					mode_pause(20);
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
