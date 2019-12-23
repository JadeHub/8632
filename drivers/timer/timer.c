#include "timer.h"
#include <kernel/x86/interrupts.h>
#include <drivers/ioports.h>
#include <drivers/console.h>
#include <kernel/utils.h>
#include <kernel/fault.h>
#include <kernel/tasks/proc.h>

#define PIT_CMD_PORT 0x43
#define PIT_CH0_PORT 0x40

extern void on_timer(isr_state_t*);

#define TIMER_CALLBACK_COUNT 64

static timer_callback_t callbacks[TIMER_CALLBACK_COUNT];

static void timer_isr(isr_state_t* state)
{
	for (int i = 0; i < TIMER_CALLBACK_COUNT; i++)
	{
		if (callbacks[i] == 0)
			return;
		(*callbacks[i])();
	}
}

void timer_init(uint32_t freq)
{
	memset(callbacks, 0, sizeof(timer_callback_t) * TIMER_CALLBACK_COUNT);

	idt_register_handler(IRQ0, &timer_isr);

	uint32_t divisor = 1193180 / freq;
	outb(PIT_CMD_PORT, 0x36);

	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

	outb(PIT_CH0_PORT, l);
	outb(PIT_CH0_PORT, h);
}

void timer_add_callback(timer_callback_t cb)
{
	for (int i = 0; i < TIMER_CALLBACK_COUNT; i++)
	{
		if (callbacks[i] == 0)
		{
			callbacks[i] = cb;
			return;
		}
	}
	KPANIC("No free timer callback slots");
}
