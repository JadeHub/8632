#include "shell.h"

#include <commands/built_in.h>
#include <sys/syscall.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static shell_state_t _shell_state;

static size_t _count_char(const char* str, char c)
{
	size_t count = 0;

	const char* s = str;
	while (*s != '\0')
	{
		if (*s == c)
			count++;
		s++;
	}
	return count;
}

static params_t _parse_cmd(char* cmd)
{
	bool space = true;
	params_t result;
	result.count = 0;

	for (size_t i = 0; cmd[i] != '\0' && i < 64; i++)
	{
		if (isspace(cmd[i]))
		{
			space = true;
			cmd[i] = '\0';
		}
		else if(space)
		{
			space = false;
			result.params[result.count] = &cmd[i];
			result.count++;
		}
	}
	for (size_t i = result.count; i < 64; i++)
		result.params[i] = NULL;
	return result;
}

static bool _process_cmd(char* cmd)
{
	if (cmd[0] == 'q')
		return false;

	params_t p = _parse_cmd(cmd);

	if (strcmp(p.params[0], "ls") == 0)
	{
		ls_cmd(p.count, p.params, &_shell_state);
	}
	else if (strcmp(p.params[0], "cd") == 0)
	{
		cd_cmd(p.count, p.params, &_shell_state);
	}
	else
	{
		printf("Command \'%s\' not found.\n", p.params[0]);
	}
	return true;
}

uint32_t shell()
{
	uint32_t fd = sys_open("/dev/console/con1", 0);
	if (fd == 0xffffffff)
	{
		printf("Failed to open consol");
		return 1;
	}
	char buff[80];

	while(1)
	{
		printf("Prompt$ ");
		sys_read(fd, buff, 79);
		if (!_process_cmd(buff))
			break;
	};
	sys_close(fd);
	return 0;
}

void entry()
{
	strcpy(_shell_state.current_dir, "/");
	printf("Starting shell\n");
	uint32_t exit_code = shell();
	sys_exit(exit_code);
}