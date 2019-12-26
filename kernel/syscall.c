#include "syscall.h"

#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>
#include <kernel/memory/kheap.h>
#include <drivers/console.h>

static uint32_t syscall_alloc(heap_t* h, uint32_t size)
{
	uint32_t ret = (uint32_t)alloc(size, 0, h);
	con_printf("Allocated %x bytes at %x\n", size, ret);
	bochs_dbg();
	return ret;
}

static void syscall_print(const char* str)
{
	con_printf("Passed back %x %s\n", str, str);
}

static void syscall_print_hex(const char* str, uint32_t v)
{
	con_printf("Value (%08x) %s = %x\n", str, str, v);
}

static void syscall_exit(uint32_t code)
{
	con_printf("Exit ebx=%x\n", code);
	for (;;);
}

static void* syscalls[4] =
{
	&syscall_alloc,
	&syscall_print,
	&syscall_exit,
	&syscall_print_hex
};

void syscall_handler(isr_state_t* regs)
{
   //con_printf("syscall %x %x\n", regs->eax, regs->esp);
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