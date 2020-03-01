#include <stdlib.h>
#include <sys/syscall.h>

void* malloc(size_t sz)
{
	return sys_alloc(sz);
}