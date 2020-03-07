#include <sys/syscall.h>
#include <ctype.h>

#define SYSCALL_MALLOC 1
#define SYSCALL_SLEEP 2
#define SYSCALL_EXIT 3
#define SYSCALL_OPEN 4
#define SYSCALL_CLOSE 5
#define SYSCALL_READ 6
#define SYSCALL_PRINT_STR 7

extern uint32_t perform_syscall(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5);

#define SYSCALL1(code, param) perform_syscall(code, (uint32_t)param, 0, 0, 0, 0)
#define SYSCALL2(code, param1, param2) perform_syscall(code, (uint32_t)param1, (uint32_t)param2, 0, 0, 0)
#define SYSCALL3(code, param1, param2, param3)	\
		perform_syscall(code, (uint32_t)param1,(uint32_t)param2, (uint32_t)param3, 0, 0)
#define SYSCALL4(code, param1, param2, param3, param4)	\
		perform_syscall(code, (uint32_t)param1, (uint32_t)param2, (uint32_t)param3, (uint32_t)param4, 0)
#define SYSCALL5(code, param1, param2, param3, param4, param5)	\
		perform_syscall(code, (uint32_t)param1, (uint32_t)param2, (uint32_t)param3, (uint32_t)param4, (uint32_t)param5)

#ifdef _LIBK

extern void con_write_buff(const char* buff, size_t sz);

#endif

void* sys_alloc(size_t sz)
{
	return (void*)SYSCALL1(SYSCALL_MALLOC, sz);
}

void sys_print_str(const char* buff, uint32_t sz)
{
#ifdef _LIBK
	con_write_buff(buff, sz);
#else
	SYSCALL2(SYSCALL_PRINT_STR, buff, sz);
#endif
}

void sys_exit(uint32_t exit_code)
{
	SYSCALL1(SYSCALL_EXIT, exit_code);
}

uint32_t sys_open(const char* path, uint32_t flags)
{
	return SYSCALL2(SYSCALL_OPEN, path, flags);
}

size_t sys_read(uint32_t fd, uint8_t* buff, size_t sz)
{
	return (size_t)SYSCALL3(SYSCALL_READ, fd, buff, sz);
}

void sys_close(uint32_t fd)
{
	SYSCALL1(SYSCALL_CLOSE, fd);
}

void sys_sleep_ms(uint32_t ms)
{
	SYSCALL1(SYSCALL_SLEEP, ms);
}