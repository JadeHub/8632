#include <sys/wait.h>
#include <sys/syscall.h>

uint32_t wait_pid(uint32_t pid)
{
	return sys_wait_pid(pid);
}