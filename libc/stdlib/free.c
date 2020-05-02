#include <stdlib.h>
#include <sys/syscall.h>

void free(void* p)
{
	sys_free(p);
}