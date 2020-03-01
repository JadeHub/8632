#include <stdlib.h>
#include <sys/syscall.h>

void exit(int exit_code)
{
	sys_exit(exit_code);
}