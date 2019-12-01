#include "syscall.h"

#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>

void syscall_handler(isr_state_t regs)
{
    bochs_dbg();
}

void syscall_init()
{
    idt_register_handler(ISR_SYSCALL, &syscall_handler);
}