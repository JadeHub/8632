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
    //dbg_dump_current_stack();
    for(;;)
        ;
}

void klog_impl(const char* file, uint32_t line, KLOG_LEVEL level, const char* module, const char* format, ...)
{
    char buff[256];
    va_list args;
    va_start(args, format);
    vsprintf(buff, format, args);
    va_end(args);

   // serial_printf(SERIAL_PORT_COM1, "[%s][%s] %s\n\r", module, level == LL_ERR ? "ERR" : "INFO", buff);
  //  printf("LOG [%s][%s] %s\n\r", module, level == LL_ERR ? "ERR" : "INFO", buff);
    if (level == LL_ERR)
        bochs_dbg();
}

void fault_init()
{
    idt_register_handler(ISR13, &gp_fault_handler);
    idt_register_handler(ISR6, &bad_opcode_fault_handler);

}