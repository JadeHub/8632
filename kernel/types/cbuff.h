#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct cbuff8
{
	uint8_t* buff;
	size_t head;
	size_t tail;
	size_t len;
	bool full;
}cbuff8_t;

cbuff8_t* cbuff8_create(uint8_t* buff, size_t sz);
void cbuff8_destory(cbuff8_t*);
void cbuff8_reset(cbuff8_t*);
void cbuff8_put(cbuff8_t*, uint8_t);
bool cbuff8_get(cbuff8_t*, uint8_t* val);
bool cbuff8_empty(cbuff8_t*);
bool cbuff8_full(cbuff8_t*);
size_t cbuff8_capacity(cbuff8_t*);
size_t cbuff8_count(cbuff8_t*);

typedef struct cbuff32
{
	uint32_t* buff;
	size_t head;
	size_t tail;
	size_t len;
	bool full;
}cbuff32_t;

cbuff32_t* cbuff32_create(uint32_t* buff, size_t sz);
void cbuff32_destory(cbuff32_t*);
void cbuff32_reset(cbuff32_t*);
void cbuff32_put(cbuff32_t*, uint32_t);
bool cbuff32_get(cbuff32_t*, uint32_t* val);
bool cbuff32_empty(cbuff32_t*);
bool cbuff32_full(cbuff32_t*);
size_t cbuff32_capacity(cbuff32_t*);
size_t cbuff32_count(cbuff32_t*);