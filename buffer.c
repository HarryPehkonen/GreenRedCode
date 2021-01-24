/*
 * buffer.c
 *
 * Created: 2021-01-24 06:54:54
 *  Author: harry
 */ 

#include <avr/io.h>
#include <string.h> /* memset */
#include <stdlib.h> /* malloc, free */
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "buffer.h"

buffer_t *
buffer_create() {
	buffer_t *buffer = malloc(sizeof(buffer_t));

	buffer->count = 0;
	buffer->max = BUFFER_MAX;
	memset(buffer->data, 0, buffer->max * sizeof(uint32_t));

	return buffer;
}

uint8_t
buffer_is_empty(buffer_t *buffer) {
	return buffer->count == 0;
}

uint32_t
buffer_pop(buffer_t *buffer) {

	uint32_t item = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	
		/* check for underflow */
		if (buffer->count != 0) {
			item = buffer->data[buffer->count - 1];
			buffer->count -= 1;
		}
	}

	return item;
}

uint8_t
buffer_push(buffer_t *buffer, uint32_t item) {

	uint8_t count = 0;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		/* check for overflow */
		if (buffer->count != buffer->max) {
			buffer->data[buffer->count] = item;
			buffer->count += 1;
			count = buffer->count;
		}
	}
	
	return count;
}

void
buffer_free(buffer_t **buffer)
{
	free(*buffer);
	*buffer = NULL;
}

void
buffer_test() {
	buffer_t *buffer = buffer_create();
	if (buffer == NULL) while (1);
	if (!buffer_is_empty(buffer)) while (1);
	if (buffer_pop(buffer) != 0) while (1);
	if (!buffer_is_empty(buffer)) while (1);
	if (buffer_push(buffer, 0x55) != 1) while (1);
	if (buffer_is_empty(buffer)) while (1);
	if (buffer->count != 1) while (1);
	if (buffer_push(buffer, 0xAA) != 2) while (1);
	if (buffer->count != 2) while (1);
	if (buffer_pop(buffer) != 0xAA) while (1);
	if (buffer->count != 1) while (1);
	if (buffer_is_empty(buffer)) while (1);
	if (buffer_pop(buffer) != 0x55) while (1);
	if (!buffer_is_empty(buffer)) while (1);
	if (buffer->count != 0) while (1);
	buffer_free(&buffer);
	if (buffer != NULL) while (1);
}
