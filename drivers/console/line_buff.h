#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct line_buff
{
	char result[1024];

	size_t len;
	size_t cur_pos;
}line_buff_t;

void lb_init(line_buff_t*);
uint8_t lb_add_code(line_buff_t*, uint8_t);