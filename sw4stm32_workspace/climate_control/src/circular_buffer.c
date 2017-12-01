#include "circular_buffer.h"

// check if a circular buffer is full.
// Ok to call from pushing process
// without locking but calling from the
// popping process requires locking
int BuffIsFull(volatile CircularBuffer *CB)
{
	volatile cb_data_t *tempTail = CB->tail; // grab a snapshot of the tail so that
									// this function is atomic when called
									// from the pushing process.
	return (
			(CB->head == tempTail-1)
			||
			((CB->head == CB->data + CB->size - 1) && (tempTail == CB->data))
			);
}

int BuffIsEmpty(volatile CircularBuffer *CB)
{
	return (CB->head == CB->tail);
}

// note: This function is non-atomic.
int BuffLevel(volatile CircularBuffer *CB)
{
	volatile cb_data_t *temphead;

	if (CB->head < CB->tail) // the data wraps around zero.
	{
		temphead = CB->head + CB->size;
	}
	else
	{
		temphead = CB->head;
	}
	return temphead - CB->tail;
}

// note: This function calls BuffLevel,
// which is non-atomic.
int BuffAvail(volatile CircularBuffer *CB)
{
	// explanation of the '-1' in the calculation below:
	// HEAD always points to an empty slot in the buffer,
	// and that empty slot can only be written to if the
	// next slot is also empty (giving a valid position for
	// HEAD to move to).  Therefore, there is always 1 less
	// position available in the buffer than the number of slots.
	return CB->size - BuffLevel(CB) - 1;
}


int BuffPop(volatile CircularBuffer *CB, cb_data_t *data)
{
	if (BuffIsEmpty(CB)) return 1; // return positive value on fail.
	*data = *CB->tail;
	CB->tail++;
	if (CB->tail > CB->data + CB->size - 1)
	{
		CB->tail = CB->data;
	}
	return 0;

}

int BuffPeek(volatile CircularBuffer *CB, cb_data_t *data)
{
	if (BuffIsEmpty(CB)) return 1; // return positive value on fail.
	*data = *CB->tail;
	return 0;
}

int BuffPush(volatile CircularBuffer *CB, cb_data_t *data)
{
	if (BuffIsFull(CB)) return 1; // return positive value on fail.
	*CB->head = *data;
	CB->head++;
	if (CB->head > CB->data + CB->size - 1)
	{
		CB->head = CB->data;
	}
	return 0;
}

void BuffClear(volatile CircularBuffer *CB)
{
	CB->head = CB->tail = CB->data;
}


