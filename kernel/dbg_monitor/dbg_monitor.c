#include "dbg_monitor.h"

#include <drivers/console.h>
#include <drivers/serial/serial_io.h>
#include <kernel/utils.h>
#include <kernel/fs/fs.h>

#include <stdio.h>

#define SERIAL_BUFF_LEN 256

char cmd_buff[SERIAL_BUFF_LEN];
uint8_t cmd_buff_len = 0;

#define CR 0x0D
#define LF 0x0A
#define CTRLC 0x03

typedef void (*cmd_handler_t)(const char* params);

typedef struct command
{
	char abbrev;
	const char* text;
	cmd_handler_t handler;
} command_t;

int cmd_count = 3;

extern void threads_cmd(const char*);
extern void kernel_cmd(const char*);

fs_node_t* _cur_dir = NULL;

static bool _ls_dir_cb(struct fs_node* parent, struct fs_node* child)
{
	dbg_mon_output_line("%08d %s", child->len, child->name);
	return true;
}

static void _ls_cmd(const char* p)
{
	if (!_cur_dir)
		return;
	fs_read_dir(_cur_dir, &_ls_dir_cb);
}

static void _cd_cmd(const char* p)
{
	if (!_cur_dir)
		return;

	fs_node_t* child = fs_find_child(_cur_dir, p);
	if (child)
		_cur_dir = child;
}

command_t _commands[] = 
{ 
	{'t', "threads", &threads_cmd},
	{'k', "kernel", &kernel_cmd},
	{0, "ls", &_ls_cmd},
	{0, "cd", &_cd_cmd},

	{0, 0, 0}
};

static void _process_cmd(char* cmd)
{
	//seperate the params
	char* params = cmd;
	while (*params != '\0' && *params != ' ')
	{
		params++;
	}
	if (*params == ' ')
	{
		*params = '\0';
		params++;
		while (isspace(*params))
			params++;
	}
	else
	{
		params = 0;
	}
	//find the command
	for(command_t* c = _commands; c->handler != NULL; c++)
	{
		if (strcmp(cmd, c->text) == 0 ||
			cmd[0] == c->abbrev)
		{
			c->handler(params);
			return;
		}
	}
	dbg_mon_output_line("Unknonwn command %s", cmd);
}

static void print_prompt(uint16_t port)
{
	serial_printf(port, "<dbg:%s>", _cur_dir->name);
}

static void reset_cmd_buff()
{
	cmd_buff[0] = '\0';
	cmd_buff_len = 0;
}

static void serial_handler(uint16_t port, uint8_t data)
{
	printf("Serial!\n");
	serial_write(port, data);

	if (data == CR)
	{
		serial_write(port, LF);
		cmd_buff[cmd_buff_len] = '\0';
		_process_cmd(cmd_buff);
		reset_cmd_buff();
		print_prompt(port);
	}
	else if (data == CTRLC)
	{
		reset_cmd_buff();
		serial_write(port, CR);
		serial_write(port, LF);
		print_prompt(port);
	}
	else
	{
		cmd_buff[cmd_buff_len++] = data;
		if (cmd_buff_len == SERIAL_BUFF_LEN-1)
		{
			reset_cmd_buff();
			dbg_mon_output_line("Buffer overflow");
		}
	}
}

void dbg_mon_init()
{
	_cur_dir = fs_root();
	reset_cmd_buff();
	serial_read(SERIAL_PORT_COM1, &serial_handler);
	dbg_mon_output_line("");
	print_prompt(SERIAL_PORT_COM1);
}

void dbg_mon_output_line(const char* format, ...)
{
	char buff[SERIAL_BUFF_LEN];
	va_list args;
	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);

	serial_write_str(SERIAL_PORT_COM1, buff);
	serial_write_str(SERIAL_PORT_COM1, "\n\r");
}

