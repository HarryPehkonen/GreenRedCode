/*
 * queue.c
 *
 * Created: 2021-01-24 06:54:54
 *  Author: harry
 */ 

#include <avr/io.h>
#include <string.h> /* memset */
#include <stdlib.h> /* malloc, free */
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "queue.h"

queue_t *
queue_create() {
	queue_t *queue = malloc(sizeof(queue_t));

	queue->count = 0;
	queue->head = 0;
	queue->tail = 0;
	queue->max = QUEUE_MAX;
	memset(queue->data, 0, queue->max * sizeof(uint32_t));

	return queue;
}

uint8_t
queue_is_empty(queue_t *queue) {
	return queue->count == 0;
}

uint32_t
queue_dequeue(queue_t *queue) {

	uint32_t item = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		/* check for underflow */
		if (queue->count != 0) {
			item = queue->data[queue->head];
			queue->head = (queue->head + 1) % queue->max;
			queue->count -= 1;
		}
	}

	return item;
}

uint8_t
queue_enqueue(queue_t *queue, uint32_t item) {

	uint8_t count = 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		/* check for overflow */
		if (queue->count != queue->max) {
			queue->data[queue->tail] = item;
			queue->tail = (queue->tail + 1) % queue->max;
			queue->count += 1;
			count = queue->count;
		}
	}

	return count;
}

void
queue_free(queue_t **queue)
{
	free(*queue);
	*queue = NULL;
}

void
queue_test() {
	queue_t *queue = queue_create();
	if (queue == NULL) while (1);
	if (!queue_is_empty(queue)) while (1);
	if (queue_dequeue(queue) != 0) while (1);
	if (!queue_is_empty(queue)) while (1);
	if (queue_enqueue(queue, 0x55) != 1) while (1);
	if (queue_is_empty(queue)) while (1);
	if (queue->count != 1) while (1);
	if (queue_enqueue(queue, 0xAA) != 2) while (1);
	if (queue->count != 2) while (1);
	if (queue_dequeue(queue) != 0x55) while (1);
	if (queue->count != 1) while (1);
	if (queue_is_empty(queue)) while (1);
	if (queue_dequeue(queue) != 0xAA) while (1);
	if (!queue_is_empty(queue)) while (1);
	if (queue->count != 0) while (1);
	queue_free(&queue);
	if (queue != NULL) while (1);
}
