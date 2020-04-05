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

static fs_node_t* _history_node = NULL;
static list_head_t _history;

static void _destroy_history()
{
	list_head_t* child = _history.next;
	while (child != &_history)
	{
		list_head_t* next = child->next;
		item_t* item = list_entry(child, item_t, list_item);
		kfree(item->value);
		kfree(item);
		child = next;
	}
	INIT_LIST_HEAD(&_history);
}

static int32_t _open_history(fs_node_t* node, uint32_t flags)
{
	INIT_LIST_HEAD(&_history);
}

static void _close_history(fs_node_t* node)
{
	_destroy_history();
}

static size_t _read_history(fs_node_t* node, uint8_t* buff, size_t off, size_t sz)
{
	printf("read 0\n");
	list_head_t* child = _history.prev;

	size_t index = 0;
	while (child != &_history && index < off)
	{
		printf("read 0.1\n");
		child = child->prev;
		index++;
	}
	if (index != off)
		return 0;
	printf("read 1\n");
	item_t* item = list_entry(child, item_t, list_item);
	printf("read 2\n");
	printf("\nreading history off %d \n", off);

	strncpy(buff, item->value, sz+1);
	return strlen(buff) + 1;
}

static size_t _write_history(fs_node_t* node, const uint8_t* buff, size_t off, size_t sz)
{
	printf("Write history %s %d\n", (char*)buff, sz);
	item_t* item = (item_t*)kmalloc(sizeof(item_t));
	item->value = (char*)kmalloc(sz + 1);
	strcpy(item->value, (const char*)buff);
	list_add(&item->list_item, &_history);
	printf("Write history %s\n", item->value);
	return sz;
}

fs_node_t* con_his_create(const char* con_name)
{
	ASSERT(!_history_node);
	char buff[128];
	strcpy(buff, con_name);
	strcat(buff, "_");
	strcat(buff, "history");
	_history_node = fs_create_node(buff);
	_history_node->read = &_read_history;
	_history_node->write = &_write_history;
	//_history_node->open = &_open_history;
	//_history_node->close = &_close_history;
	INIT_LIST_HEAD(&_history);

	if (!fs_add_child_node(fs_get_abs_path("/dev/console", NULL), _history_node))
	{
		printf("Failed to create history\n");
		fs_destroy_node(_history_node);
		return NULL;
	}

	return _history_node;
}

void con_his_destroy()
{
	ASSERT(_history_node);
	_destroy_history();
	fs_destroy_node(_history_node);
	_history_node = NULL;
}
