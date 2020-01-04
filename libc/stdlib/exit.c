#include <stdlib.h>
#include "../syscall.h"

void exit(int exit_code)
{
	SYSCALL1(SYSCALL_EXIT, exit_code);
	//perform_syscall(3, exit_code, 0, 0, 0, 0);
}