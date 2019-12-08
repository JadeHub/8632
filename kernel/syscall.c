#include "syscall.h"

#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>
#include <kernel/memory/kheap.h>

static uint32_t syscall_alloc(heap_t* h, uint32_t size)
{
	uint32_t ret = alloc(size, 0, h);
	con_printf("Allocated %x bytes at %x\n", size, ret);
	return ret;
}

static void syscall_print(const char* str)
{
	con_printf("Passed back %x %s\n", str, str);
}

static void syscall_exit(uint32_t code)
{
	con_printf("Exit ebx=%x\n", code);
	for (;;);
}

static void* syscalls[3] =
{
	&syscall_alloc,
	&syscall_print,
	&syscall_exit
};

void syscall_handler(isr_state_t* regs)
{
   con_printf("syscall %x\n", regs->eax);
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