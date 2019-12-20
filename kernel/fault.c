#include "fault.h"
#include "utils.h"
#include <kernel/x86/interrupts.h>
#include <drivers/console.h>
#include <drivers/serial/serial_io.h>

void panic_impl(const char*, const char*, uint32_t);

static void bad_opcode_fault_handler(isr_state_t* state)
{
    con_printf("Bad op code at: %x\n", state->eip);
    KPANIC("Bad Op code");
}

static void gp_fault_handler(isr_state_t* state)
{
    con_printf("GP Fault: %x at: %x\n", state->err_code, state->eip);
    KPANIC("GP Fault");
}

void panic_impl(const char* msg, const char* file, uint32_t line)
{
    con_printf("%s at %s:%d\n", msg, file, line);
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

    serial_printf(SERIAL_PORT_COM1, "[%s][%s] %s\n\r", module, level == LL_ERR ? "ERR" : "INFO", buff);
}

void fault_init()
{
    idt_register_handler(ISR13, &gp_fault_handler);
    idt_register_handler(ISR6, &bad_opcode_fault_handler);

}