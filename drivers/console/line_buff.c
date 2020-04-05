#include "line_buff.h"
#include <sys/key_codes.h>

#include <kernel/fs/node.h>
#include <kernel/fault.h>
#include <kernel/debug.h>

#include <string.h>
#include <stdio.h>

static void _insert(line_buff_t* lb, char code)
{
	ASSERT(lb->cur_pos <= lb->len);
	
	if (lb->len == 1024)
		return;
	if (lb->cur_pos < lb->len)
		for (size_t i = lb->len+1; i > lb->cur_pos; i--)
			lb->result[i] = lb->result[i-1];

	lb->result[lb->cur_pos] = code;
	lb->cur_pos++;
	lb->len++;
}

static void _remove(line_buff_t* lb)
{
	for (size_t i = lb->cur_pos; i <= lb->len; i++)
		lb->result[i] = lb->result[i + 1];
	lb->len--;
}

void lb_init(line_buff_t* lb)
{
	memset(lb, 0, sizeof(line_buff_t));
}

uint8_t lb_add_code(line_buff_t* lb, uint8_t code, fs_node_t* history)
{
	if (code == KEY_NEW_LINE)
	{
		return 0xFF;
	}
	else if (code == KEY_BACKSPACE)
	{
		if (lb->cur_pos == 0)
			return 0;
		lb->cur_pos--;
		_remove(lb);
		return KEY_BACKSPACE;
	}
	else if (code == KEY_DEL)
	{
		_remove(lb);
		return KEY_DEL;
	}
	else if (code == KEY_LEFT)
	{
		if (lb->cur_pos == 0)
			return 0;
		lb->cur_pos--;
		return KEY_LEFT;
	}
	else if (code == KEY_RIGHT)
	{
		if (lb->cur_pos == lb->len)
			return 0;
		lb->cur_pos++;
		return KEY_RIGHT;
	}
	_insert(lb, code);
	return code;
}

void lb_set(line_buff_t* lb, const char* str)
{
	strcpy(lb->result, str);
	lb->len = lb->cur_pos = strlen(str);

	//printf("Set result %s %d\n", lb->result, lb->len);
}