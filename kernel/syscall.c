#include "syscall.h"

#include <sys/signal_defs.h>

#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>
#include <kernel/memory/kheap.h>
#include <drivers/console/console.h>
#include <kernel/time.h>
#include <kernel/io/io.h>
#include <kernel/signals/signal.h>
#include <dirent.h>

#include <stdio.h>

static uint32_t syscall_alloc(isr_state_t* regs, heap_t* h, uint32_t size)
{
	uint32_t ret = (uint32_t)heap_alloc(size, 0, h);
	bochs_dbg();
	return ret;
}

static void _syscall_sleep_ms(isr_state_t* regs, uint32_t ms)
{
    sched_sleep_until(time_ms() + ms);
}

static void _syscall_print_str(isr_state_t* regs, const char* buff, uint32_t sz)
{
    con_write_buff(buff, sz);
}

static void _syscall_exit(isr_state_t* regs, int32_t code)
{
    sched_exit(code);
}

static uint32_t _syscall_open(isr_state_t* regs, const char* path, uint32_t flags)
{
    return io_open(path, flags);
}

static void _syscall_close(isr_state_t* regs, uint32_t fd)
{
    io_close(fd);
}

static size_t _syscall_write(isr_state_t* regs, uint32_t fd, uint8_t* buff, size_t sz)
{
    return io_write(fd, buff, sz);
}

static size_t _syscall_read(isr_state_t* regs, uint32_t fd, uint8_t* buff, size_t sz)
{
    return io_read(fd, buff, sz);
}

static dirent_t* _syscall_read_dir(isr_state_t* regs, struct DIR* dir)
{
    return io_readdir(dir);
}

static void _syscall_close_dir(isr_state_t* regs, struct DIR* dir)
{
    io_closedir(dir);
}

static struct DIR* _syscall_open_dir(isr_state_t* regs, const char* path)
{
    return io_opendir(path);
}

static uint32_t _syscall_start_proc(isr_state_t* regs, const char* path, const char* args[], uint32_t fds[3])
{
    return proc_start_user_proc(path, args, fds);
}

static uint32_t _syscall_wait_pid(isr_state_t* regs, uint32_t pid)
{
    return proc_wait_pid(pid);
}

static void _syscall_register_sig_handler(isr_state_t* regs, int sig, sig_handler_t handler)
{
    sig_set_handler(sched_cur_proc(), sig, handler);
   // sig_queue_signal(sched_cur_proc(), sig);
}

static void _syscall_sig_handler_return(isr_state_t* regs)
{
    sig_return(sched_cur_proc(), regs);
}

static bool _syscall_sig_send_signal(uint32_t pid, uint32_t sig)
{
    return sig_queue_signal(proc_get_pid(pid), sig);
}

static void* syscalls[15] =
{
	&syscall_alloc,
	&_syscall_sleep_ms,
	&_syscall_exit,
    &_syscall_open,
    &_syscall_close,
    &_syscall_read,
	&_syscall_write,
    &_syscall_read_dir,
    &_syscall_open_dir,
    &_syscall_close_dir,
    &_syscall_start_proc,
    &_syscall_wait_pid,
    &_syscall_register_sig_handler,
    &_syscall_sig_handler_return,
    &_syscall_sig_send_signal
};

void syscall_handler(isr_state_t* regs)
{
   //printf("syscall 0x%x 0x%x\n", regs->eax, regs->esp);
   void *location = syscalls[regs->eax-1];

   typedef uint32_t(*sys_handler_t)(isr_state_t*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

   sys_handler_t fn = (sys_handler_t)location;
   regs->eax = fn(regs, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
}

void syscall_init()
{
    idt_register_handler(ISR_SYSCALL, &syscall_handler);
}