#include "timer.h"
#include <kernel/x86/interrupts.h>
#include <drivers/ioports.h>
#include <drivers/console.h>

#define PIT_CMD_PORT 0x43
#define PIT_CH0_PORT 0x40

static void timer_isr(isr_state_t state)
{
    //con_write("Timer\n");
}

void timer_init(uint32_t freq)
{
    idt_register_handler(IRQ0, &timer_isr);

    uint32_t divisor = 1193180 / freq;
    outb(PIT_CMD_PORT, 0x36);

   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   outb(PIT_CH0_PORT, l);
   outb(PIT_CH0_PORT, h);
}
    