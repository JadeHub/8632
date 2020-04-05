#pragma once

#include <kernel/types/list.h>

typedef struct con_history
{
	list_head_t list;
	size_t len;
}con_history_t;

con_history_t* con_his_create();
void con_his_destroy(con_history_t*);
const char* con_his_get(con_history_t*, size_t);
void con_his_add(con_history_t*, const char*);
