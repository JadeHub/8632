#include "cbuff.h"

#include <kernel/memory/kmalloc.h>

cbuff_t* cbuff_create(uint8_t* buff, size_t sz)
{
	cbuff_t* b = (cbuff_t*)kmalloc(sizeof(cbuff_t));
	b->buff = buff;
	b->len = sz;
	cbuff_reset(b);
	return b;
}

void cbuff_destory(cbuff_t* b)
{
	kfree(b);
}

void cbuff_reset(cbuff_t* b)
{
	b->full = false;
	b->head = b->tail = 0;
}

void cbuff_put(cbuff_t* b, uint8_t v)
{
	b->buff[b->head] = v;

	if (b->full)
		b->tail = (b->tail + 1) % b->len;
	b->head = (b->head + 1) % b->len;
	b->full = b->head == b->tail;
}

bool cbuff_get(cbuff_t* b, uint8_t* val)
{
	if (cbuff_empty(b))
		return false;
	*val = b->buff[b->tail];
	b->full = false;
	b->tail = (b->tail + 1) % b->len;
}

bool cbuff_empty(cbuff_t* b)
{
	return !b->full && b->head == b->tail;
}

bool cbuff_full(cbuff_t* b)
{
	return b->full;
}

size_t cbuff_capacity(cbuff_t* b)
{
	return b->len;
}

size_t cbuff_count(cbuff_t* b)
{
	if (b->full)
		return b->len;
	if (b->head > b->tail)
		return b->head - b->tail;
	return b->len - b->tail + b->head;
}
