#include "shell.h"
#include "built_in.h"

#include <stdio.h>

void clear_cmd(size_t count, const char* params[], shell_state_t* shell)
{
	printf("\033c");

}