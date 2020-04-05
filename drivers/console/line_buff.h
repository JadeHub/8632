#pragma once

#include "history.h"

#include <stdint.h>
#include <stddef.h>

typedef struct line_buff
{
	char result[1024];

	size_t len;
	size_t cur_pos;
	size_t history_pos;
}line_buff_t;

void lb_init(line_buff_t*);

typedef enum {REPLACE, IGNORE, APPEND, BREAK} lb_result_t;

lb_result_t lb_add_code(line_buff_t*, uint8_t, con_history_t*);