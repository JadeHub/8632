#include <sys/syscall.h>
#include <ctype.h>

#define SYSCALL_MALLOC 1
#define SYSCALL_SLEEP 2
#define SYSCALL_EXIT 3
#define SYSCALL_OPEN 4
#define SYSCALL_CLOSE 5
#define SYSCALL_READ 6
#define SYSCALL_WRITE 7
#define SYSCALL_READ_DIR 8
#define SYSCALL_OPEN_DIR 9
#define SYSCALL_CLOSE_DIR 10
#define SYSCALL_START_PROC 11
#define SYSCALL_WAIT_PID 12
#define SYSCALL_REG_SIG_HANDLER 13
#define SYSCALL_SIG_HANDLER_RET 14
#define SYSCALL_SIG_SEND 15

extern uint32_t perform_syscall(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5);

#define SYSCALL0(code) perform_syscall(code, 0, 0, 0, 0, 0)
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
	sys_write(1, buff, sz);
#endif
}

void sys_exit(int32_t exit_code)
{
	SYSCALL1(SYSCALL_EXIT, exit_code);
}

uint32_t sys_open(const char* path, uint32_t flags)
{
	return SYSCALL2(SYSCALL_OPEN, path, flags);
}

size_t sys_write(uint32_t fd, const uint8_t* buff, size_t sz)
{
	return (size_t)SYSCALL3(SYSCALL_WRITE, fd, buff, sz);
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

void sys_read_dir(const char* path, void(fn)(const char*))
{
	SYSCALL2(SYSCALL_READ_DIR, path, fn);
}

struct DIR* sys_opendir(const char* path)
{
	return (struct DIR*)SYSCALL1(SYSCALL_OPEN_DIR, path);
}

void sys_closedir(struct DIR* dir)
{
	SYSCALL1(SYSCALL_CLOSE_DIR, dir);
}

struct dirent* sys_readdir(struct DIR* dir)
{
	return (struct dirent*)SYSCALL1(SYSCALL_READ_DIR, dir);
}
uint32_t sys_start_proc(const char* path, const char* args[], uint32_t fds[3])
{
	return SYSCALL3(SYSCALL_START_PROC, path, args, fds);
}

uint32_t sys_wait_pid(uint32_t pid)
{
	return SYSCALL1(SYSCALL_WAIT_PID, pid);
}

void sys_reg_sig_handler(int sig, sig_handler_t handler)
{
	SYSCALL2(SYSCALL_REG_SIG_HANDLER, sig, handler);
}

void sys_sig_handler_return()
{
	SYSCALL0(SYSCALL_SIG_HANDLER_RET);
}

bool sys_send_signal(uint32_t pid, uint32_t sig)
{
	return SYSCALL2(SYSCALL_SIG_SEND, pid, sig);
}