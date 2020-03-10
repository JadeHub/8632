#include "timer.h"
#include <kernel/x86/interrupts.h>
#include <drivers/ioports.h>
#include <kernel/utils.h>

#define PIT_CMD_PORT 0x43
#define PIT_CH0_PORT 0x40

static timer_callback_t _cb = NULL;
static uint64_t _ms_since_boot = 0;
static uint64_t _ms_per_tick;

static void timer_isr(isr_state_t* state)
{
	_ms_since_boot += _ms_per_tick;
	_cb(_ms_since_boot);
}

void timer_init(uint32_t freq, timer_callback_t cb)
{
	_cb = cb;
	_ms_per_tick = 1000 / freq; //1000 ms per sec / ticks per sec

	idt_register_handler(IRQ0, &timer_isr);

	uint32_t divisor = 1193180 / freq;
	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );
	outb(PIT_CMD_PORT, 0x36);
	outb(PIT_CH0_PORT, l);
	outb(PIT_CH0_PORT, h);
}
