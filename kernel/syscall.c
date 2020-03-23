#include "syscall.h"

#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>
#include <kernel/memory/kheap.h>
#include <drivers/console/console.h>
#include <kernel/time.h>
#include <kernel/io/io.h>

#include <stdio.h>

static uint32_t syscall_alloc(heap_t* h, uint32_t size)
{
	uint32_t ret = (uint32_t)alloc(size, 0, h);
	bochs_dbg();
	return ret;
}

static void _syscall_sleep_ms(uint32_t ms)
{
    sched_sleep_until(time_ms() + ms);
}

static void _syscall_print_str(const char* buff, uint32_t sz)
{
    con_write_buff(buff, sz);
}

static void syscall_exit(uint32_t code)
{
	printf("Exit ebx=0x%x\n", code);
    sched_exit(code);
//	for (;;);
}

static uint32_t _syscall_open(const char* path, uint32_t flags)
{
    return open(path, flags);
}

static void _syscall_close(uint32_t fd)
{
    close(fd);
}

static size_t _syscall_read(uint32_t fd, uint8_t* buff, size_t sz)
{
    return read(fd, buff, sz);
}

static void* syscalls[7] =
{
	&syscall_alloc,
	&_syscall_sleep_ms,
	&syscall_exit,
    &_syscall_open,
    &_syscall_close,
    &_syscall_read,
	&_syscall_print_str
};

void syscall_handler(isr_state_t* regs)
{
   //printf("syscall 0x%x 0x%x\n", regs->eax, regs->esp);
   void *location = syscalls[regs->eax-1];

   // We don't know how many parameters the function wants, so we just
   // push them all onto the stack in the correct order. The function will
   // use all the parameters it wants, and we can pop them all back off afterwards.
   int ret;
   asm volatile (" \
     push %1; \
     push %2; \
     push %3; \
     push %4; \
     push %5; \
     call *%6; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
   " : "=a" (ret) :
   "r" (regs->edi),
   "r" (regs->esi),
   "r" (regs->edx),
   "r" (regs->ecx),
   "r" (regs->ebx),
   "r" (location));
   regs->eax = ret;
}

void syscall_init()
{
    idt_register_handler(ISR_SYSCALL, &syscall_handler);
}