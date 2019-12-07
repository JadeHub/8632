#include "syscall.h"

#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>
#include <kernel/memory/kheap.h>

void syscall_handler(isr_state_t* regs)
{
   con_printf("syscall %x\n", regs->eax);

	if(regs->eax == 1)
	{
		heap_t* h =(heap_t*)regs->ebx;
		uint32_t ret = alloc(regs->ecx, 0, h);
		con_printf("Allocated %x bytes at %x\n", regs->ecx, ret);
		regs->eax = ret;
	}
	if (regs->eax == 2)
	{
		char* str = (char*)regs->ebx;
		con_printf("Passed back %x %s\n", regs->ebx, str);
	}
	if (regs->eax == 3)
	{
		con_printf("Exit ebx=%x\n", regs->ebx);
		for(;;);
	}
}

void syscall_init()
{
    idt_register_handler(ISR_SYSCALL, &syscall_handler);
}