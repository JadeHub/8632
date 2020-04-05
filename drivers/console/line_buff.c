#include "line_buff.h"
#include "history.h"
#include <sys/key_codes.h>


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

lb_result_t lb_add_code(line_buff_t* lb, uint8_t code, con_history_t* history)
{
	if (code == KEY_NEW_LINE)
	{
		return BREAK;
	}
	else if (code == KEY_UP)
	{
		if (lb->history_pos == history->len)
			return IGNORE;
		lb->history_pos++;
		const char* str = con_his_get(history, lb->history_pos-1);
		strcpy(lb->result, str);
		lb->len = lb->cur_pos = strlen(str);
		return REPLACE;
	}
	else if (code == KEY_DOWN)
	{
		if (lb->history_pos == 0)
			return IGNORE;
		lb->history_pos--;
		if (lb->history_pos > 0)
		{
			const char* str = con_his_get(history, lb->history_pos - 1);
			strcpy(lb->result, str);
			lb->len = lb->cur_pos = strlen(str);
		}
		else
		{
			lb->result[0] = '\0';
			lb->len = lb->cur_pos = 0;
		}
		return REPLACE;
	}
	else if (code == KEY_BACKSPACE)
	{
		if (lb->cur_pos == 0)
			return 0;
		lb->cur_pos--;
		_remove(lb);
		return REPLACE;
	}
	else if (code == KEY_DEL)
	{
		if (lb->cur_pos < lb->len)
		{
			_remove(lb);
			return REPLACE;
		}
		return IGNORE;
	}
	else if (code == KEY_LEFT)
	{
		if (lb->cur_pos == 0)
			return 0;
		lb->cur_pos--;
		return IGNORE;
	}
	else if (code == KEY_RIGHT)
	{
		if (lb->cur_pos == lb->len)
			return 0;
		lb->cur_pos++;
		return IGNORE;
	}
	_insert(lb, code);
	return APPEND;
}
