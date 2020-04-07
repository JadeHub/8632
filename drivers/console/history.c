#include "history.h"

#include <kernel/fs/fs.h>
#include <kernel/fault.h>
#include <kernel/types/list.h>
#include <kernel/memory/kmalloc.h>

#include <stdio.h>
#include <string.h>

typedef struct item
{
	list_head_t list_item;
	char* value;
}item_t;

static list_head_t _history;

con_history_t* con_his_create()
{
	con_history_t* result = (con_history_t * )kmalloc(sizeof(con_history_t));
	INIT_LIST_HEAD(&result->list);
	result->len = 0;
	return result;
}

void con_his_destroy(con_history_t* history)
{
	list_head_t* child = history->list.next;
	while (child != &history->list)
	{
		list_head_t* next = child->next;
		item_t* item = list_entry(child, item_t, list_item);
		kfree(item->value);
		kfree(item);
		child = next;
	}
	kfree(history);
}

const char* con_his_get(con_history_t* history, size_t index)
{
	list_head_t* child = history->list.next;

	size_t i = 0;
	while (child != &history->list && i < index)
	{
		child = child->next;
		i++;
	}
	if (i != index)
		return NULL;
	item_t* item = list_entry(child, item_t, list_item);
	return item->value;
}

void con_his_add(con_history_t* history, const char* str)
{
	item_t* item = (item_t*)kmalloc(sizeof(item_t));
	item->value = (char*)kmalloc(strlen(str) + 1);
	strcpy(item->value, str);
	list_add(&item->list_item, &history->list);
	history->len++;
}