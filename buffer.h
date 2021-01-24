/*
 * buffer.h
 *
 * Created: 2021-01-24 06:54:39
 *  Author: harry
 */ 


#ifndef BUFFER_H_
#define BUFFER_H_

#define BUFFER_MAX 32

typedef struct buffer_t {
	uint8_t count;
	uint8_t max;
	uint32_t data[BUFFER_MAX];
} buffer_t;

buffer_t *buffer_create();
uint8_t   buffer_is_empty(buffer_t *buffer);
uint32_t  buffer_pop(buffer_t *buffer);
uint8_t   buffer_push(buffer_t *buffer, uint32_t item);
void      buffer_free(buffer_t **buffer);

void      buffer_test();

#endif /* BUFFER_H_ */