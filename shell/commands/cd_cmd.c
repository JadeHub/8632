#include "shell.h"
#include "built_in.h"

#include <stdio.h>

void cd_cmd(size_t count, const char* params[], shell_state_t* shell)
{
	char* t = (char*)0xcccc0000;
	*t = 'j';
	printf("test\n");
}