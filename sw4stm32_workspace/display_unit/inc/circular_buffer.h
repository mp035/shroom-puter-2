#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>
#include <stdbool.h>


// the datatype of the buffer can
// be changed by changing this typedef
typedef uint8_t cb_data_t;

typedef struct
{
	volatile cb_data_t *data;
	volatile cb_data_t *head;
	volatile cb_data_t *tail;
	volatile cb_data_t size;
} CircularBuffer;

// this macro allows us to statically define/declare circular buffers
// of varying sizes without malloc/free
#define NEW_CIRC_BUFFER(name, bufferSize) volatile cb_data_t name##DataBuffer[bufferSize];\
									volatile CircularBuffer name = { \
											.data = name##DataBuffer,\
											.head = name##DataBuffer,\
											.tail = name##DataBuffer,\
											.size = bufferSize\
											}



int BuffPeek(volatile CircularBuffer *CB, cb_data_t *data);
int BuffPop(volatile CircularBuffer *CB, cb_data_t *data);
int BuffPush(volatile CircularBuffer *CB, cb_data_t *data);

// ***************************************
// check if a circular buffer is full.
// This function is ok to call from the
// pushing process without locking but
// calling from the popping process
// requires locking
int BuffIsFull(volatile CircularBuffer *CB);
// **************************************

int BuffIsEmpty(volatile CircularBuffer *CB);
// *********************************************
// NOTE: These two functions are non-atomic,
// you may need to disable interrupts or take
// similar precautions before calling them.
int BuffLevel(volatile CircularBuffer *CB);
int BuffAvail(volatile CircularBuffer *CB);
// *********************************************
void BuffClear(volatile CircularBuffer *CB);

#endif
