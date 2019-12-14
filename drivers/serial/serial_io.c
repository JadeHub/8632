#include "serial_io.h"
#include <drivers/ioports.h>
#include <kernel/x86/interrupts.h>
#include <drivers/console.h>
#include <kernel/utils.h>

#define COM1_PORT 0x3F8

static void serial_isr(isr_state_t* state)
{
	bochs_dbg();
	con_write("serial isr\n");
}

int is_transmit_empty(uint16_t port)
{
	return inb(port + 5) & 0x20;
}

static void write_char(uint16_t port, char c)
{
	while (is_transmit_empty(port) == 0);

	outb(port, c);
}

static void init_port(uint16_t port)
{
	if(port == SERIAL_PORT_COM1)
		idt_register_handler(IRQ4, &serial_isr);

	outb(port + 1, 0x00);    // Disable all interrupts
	outb(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outb(port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(port + 1, 0x00);    //                  (hi byte)
	outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit
	outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static void write_str(uint16_t port, const char* string)
{
	const char* c = string;
	while (*c)
	{
		write_char(port, *c);
		c++;
	}
}

void serial_init()
{
	init_port(SERIAL_PORT_COM1);
}

void serial_printf(uint16_t port, const char* format, ...)
{
	char buff[128];
	va_list args;
	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);
	write_str(port, buff);
}
