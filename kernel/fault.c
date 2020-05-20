#include "fault.h"
#include "debug.h"
#include "utils.h"
#include <kernel/x86/interrupts.h>
#include <drivers/serial/serial_io.h>

#include <stdio.h>

void panic_impl(const char*, const char*, uint32_t);

static void bad_opcode_fault_handler(isr_state_t* state)
{
    printf("Bad op code at: 0x%x\n", state->eip);
    KPANIC("Bad Op code");
}

static void gp_fault_handler(isr_state_t* state)
{
    printf("GP Fault: 0x%x at: 0x%x\n", state->err_code, state->eip);
    KPANIC("GP Fault");
}

static int _j = 0;

void panic_impl(const char* msg, const char* file, uint32_t line)
{
    printf("KPANIC %s at %s:%d\n", msg, file, line);
    dbg_dump_current_stack();
    for(;;)
        ;
}

void fault_init()
{
    idt_register_handler(ISR13, &gp_fault_handler);
    idt_register_handler(ISR6, &bad_opcode_fault_handler);

}