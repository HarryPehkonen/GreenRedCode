/*
 * sirc.h
 *
 * Created: 2021-01-17 09:14:55
 *  Author: harry
 */ 


#ifndef SIRC_H_
#define SIRC_H_

typedef uint32_t message_t;

void sirc_edge(uint16_t ticks);
void sirc_set_on_message(void (*fn)(message_t));

void sirc_test();

#endif /* SIRC_H_ */