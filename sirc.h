/*
 * sirc.h
 *
 * Created: 2021-01-17 09:14:55
 *  Author: harry
 */ 


#ifndef SIRC_H_
#define SIRC_H_

/*
 * using this type for sirc_code_t means no need to lock nor malloc/free
 * memory in downstream applications (i.e. queue).
 */
typedef uint32_t sirc_code_t;

/* 8 bits per byte */
/* SIRC code length fits inside 5 MSBs */
#define SIRC_CODE_MASK   0b00000111111111111111111111111111
#define SIRC_LENGTH_MASK (~(SIRC_CODE_MASK))
#define SIRC_GET_CODE(c) ((c) & SIRC_CODE_MASK)
#define SIRC_LENGTH_SHIFT_AMOUNT ((sizeof(sirc_code_t) * 8) - 5)
#define SIRC_GET_LENGTH(c) ((c) >> SIRC_LENGTH_SHIFT_AMOUNT)

void sirc_edge(uint16_t ticks);
void sirc_set_on_code(void (*fn)(sirc_code_t));

#ifdef DEBUG
void sirc_test();
#endif

#endif /* SIRC_H_ */