#include "shell.h"
#include "built_in.h"

#include <sys/syscall.h>
#include <stdio.h>

void rm_cmd(size_t count, const char* params[], shell_state_t* shell)
{
	if (count != 2)
	{
		printf("rm path\n");
		return;
	}
	const char* path = params[1];
	if (sys_remove(path) < 0)
	{
		printf("Error removing %s\n", path);
	}
}