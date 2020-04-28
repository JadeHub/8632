#include "shell.h"

#include <commands/built_in.h>
#include <sys/syscall.h>
#include <sys/wait.h>

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
	else if (strcmp(p.params[0], "rm") == 0)
	{
		rm_cmd(p.count, p.params, &_shell_state);
	}
	else if (strcmp(p.params[0], "cd") == 0)
	{
		cd_cmd(p.count, p.params, &_shell_state);
	}
	else if (strcmp(p.params[0], "clear") == 0)
	{
		clear_cmd(p.count, p.params, &_shell_state);
	//	printf("\033c");
	}
	else if (strcmp(p.params[0], "tw") == 0)
	{
		uint8_t buff[10];
		uint32_t fd = sys_open("fatfs/test.txt", 0);
		//size_t s = sys_write(fd, "test", 4);
		//printf("Opened %d wrote %d \n", fd, s);

		size_t s = sys_read(fd, buff, 5);
		printf("Read %d bytes %s\n", s, buff);
		sys_close(fd);
	}
	else if (strcmp(p.params[0], "test") == 0)
	{
		uint32_t fds[3];
		fds[0] = 0xffffffff;
		fds[1] = 0xffffffff;
		fds[2] = 0xffffffff;
		uint32_t pid = sys_start_proc("/initrd/bin/user_space", p.params, fds);
		if (pid == 0)
		{
			printf("Failed to exec\n");
		}
		else
		{
			uint32_t code = wait_pid(pid);
			printf("Prog exited with code %d\n", code);
		}
	}
	else if (strlen(cmd) > 0)
	{
		printf("Command \'%s\' not found.\n", p.params[0]);
	}
	return true;
}

static size_t _read_line(char* buff, size_t sz, uint32_t fd)
{
	if (sz == 0)
		return 0;

	size_t i = 0;
	for (; i < sz - 1; i++)
	{
		sys_read(fd, buff + i, 1);
		if (buff[i] == '\n')
			break;
	}
	buff[i++] = '\0';
	return i;
}

uint32_t shell()
{
//	uint32_t fd = sys_open("/dev/console/con1", 0);
	uint32_t fd = 0;
	if (fd == 0xffffffff)
	{
		printf("Failed to open consol");
		return 1;
	}
	char buff[80];

	while(1)
	{
		printf("Prompt$ ");
		_read_line(buff, 80, fd);
		if (!_process_cmd(buff))
			break;
	};
	//sys_close(fd);
	return 0;
}

int main(int argc, char* argv[])
{
	strcpy(_shell_state.current_dir, "/");
	printf("Starting shell\n");
	return shell();
}