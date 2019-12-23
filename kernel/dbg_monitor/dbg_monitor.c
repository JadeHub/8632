#include "dbg_monitor.h"

#include <drivers/console.h>
#include <drivers/serial/serial_io.h>
#include <kernel/utils.h>

#define SERIAL_BUFF_LEN 256

char cmd_buff[SERIAL_BUFF_LEN];
uint8_t cmd_buff_len = 0;

#define CR 0x0D
#define LF 0x0A
#define CTRLC 0x03

typedef void (*cmd_handler_t)(const char* params);

typedef struct command
{
	const char* text;
	cmd_handler_t handler;
} command_t;

int cmd_count = 2;

extern void threads_cmd(const char* params);

command_t commands[] = 
{ 
	{"threads", &threads_cmd},
	{"mem", 0}
};

static void process_cmd(char* cmd)
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
	}
	//find the command
	for (int i = 0; i < cmd_count; i++)
	{
		if (strcmp(cmd, commands[i].text) == 0 ||
			cmd[0] == commands[i].text[0]) //match on first char for now
		{
			commands[i].handler(params);
			return;
		}
	}
	dbg_mon_output_line("Unknonwn command");
}

static void print_prompt(uint16_t port)
{
	serial_printf(port, "<dbg:>");
}

static void reset_cmd_buff()
{
	cmd_buff[0] = '\0';
	cmd_buff_len = 0;
}

static void serial_handler(uint16_t port, uint8_t data)
{
	serial_write(port, data);

	if (data == CR)
	{
		serial_write(port, LF);
		cmd_buff[cmd_buff_len] = '\0';
		process_cmd(cmd_buff);
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

