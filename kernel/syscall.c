#include "syscall.h"

#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>
#include <kernel/memory/kheap.h>

void syscall_handler(isr_state_t regs)
{
   con_write("syscall ");
   con_write_hex(regs.eax);
   con_write("\n");

	if(regs.eax == 1)
	{
		heap_t* h =(heap_t*)regs.ebx;
		uint32_t ret = alloc(regs.ecx, 0, h);
		con_write("Allocated at ");
		con_write_hex(ret);
		con_write("\n");
		regs.eax = ret;
	}
	if (regs.eax == 2)
	{
		con_write("Passed back ");
		con_write_hex(regs.ebx);
		con_write("\n");
	}
	if (regs.eax == 3)
	{
		con_write("Exit\n");
		for(;;);
	}
}

void syscall_init()
{
    idt_register_handler(ISR_SYSCALL, &syscall_handler);
}