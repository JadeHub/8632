#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct cbuff
{
	uint8_t* buff;
	size_t head;
	size_t tail;
	size_t len;
	bool full;
}cbuff_t;

cbuff_t* cbuff_create(uint8_t* buff, size_t sz);
void cbuff_destory(cbuff_t*);
void cbuff_reset(cbuff_t*);
void cbuff_put(cbuff_t*, uint8_t);
bool cbuff_get(cbuff_t*, uint8_t* val);
bool cbuff_empty(cbuff_t*);
bool cbuff_full(cbuff_t*);
size_t cbuff_capacity(cbuff_t*);
size_t cbuff_count(cbuff_t*);
