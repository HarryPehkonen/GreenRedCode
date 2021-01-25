/*
 * queue.h
 *
 * Created: 2021-01-24 06:54:39
 *  Author: harry
 */ 


#ifndef QUEUE_H_
#define QUEUE_H_

#define QUEUE_MAX 32

typedef struct queue_t {
	uint8_t head;
	uint8_t tail;
	uint8_t count;
	uint8_t max;
	uint32_t data[QUEUE_MAX];
} queue_t;

queue_t  *queue_create();
uint8_t   queue_is_empty(queue_t *queue);
uint32_t  queue_dequeue(queue_t *queue);
uint8_t   queue_enqueue(queue_t *queue, uint32_t item);
void      queue_free(queue_t **queue);

void      queue_test();

#endif /* QUEUE_H_ */