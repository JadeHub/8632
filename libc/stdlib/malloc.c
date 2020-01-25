#include <stdlib.h>

#include "syscall.h"

void* malloc(size_t sz)
{
	return (void*)SYSCALL1(SYSCALL_MALLOC, sz);
}