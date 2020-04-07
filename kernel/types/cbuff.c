#include "cbuff.h"

#include <kernel/memory/kmalloc.h>

cbuff8_t* cbuff8_create(uint8_t* buff, size_t sz)
{
	cbuff8_t* b = (cbuff8_t*)kmalloc(sizeof(cbuff8_t));
	b->buff = buff;
	b->len = sz;
	cbuff8_reset(b);
	return b;
}

void cbuff8_destory(cbuff8_t* b)
{
	kfree(b);
}

void cbuff8_reset(cbuff8_t* b)
{
	b->full = false;
	b->head = b->tail = 0;
}

void cbuff8_put(cbuff8_t* b, uint8_t v)
{
	b->buff[b->head] = v;

	if (b->full)
		b->tail = (b->tail + 1) % b->len;
	b->head = (b->head + 1) % b->len;
	b->full = b->head == b->tail;
}

bool cbuff8_get(cbuff8_t* b, uint8_t* val)
{
	if (cbuff8_empty(b))
		return false;
	*val = b->buff[b->tail];
	b->full = false;
	b->tail = (b->tail + 1) % b->len;
	return true;
}

bool cbuff8_empty(cbuff8_t* b)
{
	return !b->full && b->head == b->tail;
}

bool cbuff8_full(cbuff8_t* b)
{
	return b->full;
}

size_t cbuff8_capacity(cbuff8_t* b)
{
	return b->len;
}

size_t cbuff8_count(cbuff8_t* b)
{
	if (b->full)
		return b->len;
	if (b->head > b->tail)
		return b->head - b->tail;
	return b->len - b->tail + b->head;
}

cbuff32_t* cbuff32_create(uint32_t* buff, size_t sz)
{
	cbuff32_t* b = (cbuff32_t*)kmalloc(sizeof(cbuff32_t));
	b->buff = buff;
	b->len = sz;
	cbuff32_reset(b);
	return b;
}

void cbuff32_destory(cbuff32_t* b)
{
	kfree(b);
}

void cbuff32_reset(cbuff32_t* b)
{
	b->full = false;
	b->head = b->tail = 0;
}

void cbuff32_put(cbuff32_t* b, uint32_t v)
{
	b->buff[b->head] = v;

	if (b->full)
		b->tail = (b->tail + 1) % b->len;
	b->head = (b->head + 1) % b->len;
	b->full = b->head == b->tail;
}

bool cbuff32_get(cbuff32_t* b, uint32_t* val)
{
	if (cbuff32_empty(b))
		return false;
	*val = b->buff[b->tail];
	b->full = false;
	b->tail = (b->tail + 1) % b->len;
	return true;
}

bool cbuff32_empty(cbuff32_t* b)
{
	return !b->full && b->head == b->tail;
}

bool cbuff32_full(cbuff32_t* b)
{
	return b->full;
}

size_t cbuff32_capacity(cbuff32_t* b)
{
	return b->len;
}

size_t cbuff32_count(cbuff32_t* b)
{
	if (b->full)
		return b->len;
	if (b->head > b->tail)
		return b->head - b->tail;
	return b->len - b->tail + b->head;
}
