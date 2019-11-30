#include "fault.h"
#include <kernel/x86/interrupts.h>
#include <drivers/console.h>

void panic_impl(const char*, const char*, uint32_t);

static void gp_fault_handler(isr_state_t state)
{
    con_write("GP Fault: ");
    con_write_hex(state.err_code);
    KPANIC("GP Fault");
}

void panic_impl(const char* msg, const char* file, uint32_t line)
{
    con_write(msg);
    con_write(" at ");
    con_write(file);
    con_write(":");
    con_write_hex(line);
    con_write("\n");
    for(;;)
        ;
}

void fault_init()
{
    idt_register_handler(ISR13, &gp_fault_handler);
}