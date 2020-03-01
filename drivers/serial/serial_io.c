#include "serial_io.h"
#include <drivers/ioports.h>
#include <kernel/x86/interrupts.h>
#include <drivers/console.h>
#include <kernel/utils.h>
#include <kernel/fault.h>

#define INTERRUPT_EN_REG 1
#define INTERRUPT_ID_REG 2
#define FIFO_CTRL_REG 2
#define LINE_CTLR_REG 3
#define MODEM_CTRL_REG 4
#define LINE_STATUS_REG 5
#define MODEM_STATUS_REG 6

#define LSR_DATA_READY 1
#define LSR_OVERRUN_ERR 2
#define LSR_PARITY_ERR 4
#define LSR_FRAMING_ERR 8
#define LSR_BREAK_INT 16
#define LSR_EMPTY_TX 32
#define LSR_EMPTY_RX 64
#define LSR_FIFO_ERR 128

serial_callback_t com1_handler = 0;

static uint8_t line_status(uint16_t port)
{
	return inb(port + LINE_STATUS_REG);
}

static void handle_line_status(uint16_t port)
{
	uint8_t status = line_status(port);
	KLOG(LL_INFO, "Serial", "Port 0x%x status 0x%x", port, status);
	
	bochs_dbg();
}

static void handle_interrupt(uint16_t port)
{
	uint8_t interrupt = inb(port + INTERRUPT_ID_REG);
	if (interrupt & 1)
		return; //interrupt for different port
	//strip off the reserved and fifo related bits
	switch (interrupt & 0x0F)
	{
	case (0x00):
		//Modem status ?
		inb(port + MODEM_STATUS_REG);
		break;
	case (0x02):
		//ready to write
		//printf("Serial ready to write\n");
		break;
	case (0x04):
		//data available to read
		while (line_status(port) & LSR_DATA_READY)
		{
			uint8_t data = inb(port);
			if (com1_handler)
				com1_handler(SERIAL_PORT_COM1, data);
			//printf("test %c 0x%x\n", data, data);
		}
		break;
	case (0x06):
		//line status
		handle_line_status(port);
		break;
	}	
}

static void serial_isr(isr_state_t* state)
{
	handle_interrupt(SERIAL_PORT_COM1);
	//COM2 ...
}
	
static int is_transmit_empty(uint16_t port)
{
	return line_status(port) & LSR_EMPTY_TX;
}

void serial_write(uint16_t port, uint8_t c)
{
	while (is_transmit_empty(port) == 0);

	outb(port, c);
}

static void init_port(uint16_t port)
{
	if(port == SERIAL_PORT_COM1)
		idt_register_handler(IRQ4, &serial_isr);

	outb(port + INTERRUPT_EN_REG, 0x00);    // Disable all interrupts
	outb(port + LINE_CTLR_REG, 0x80);    // Enable DLAB (set baud rate divisor)
	outb(port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(port + 1, 0x00);    //                  (hi byte)
	outb(port + LINE_CTLR_REG, 0x03);    // Disable DLAB, 8 bits, no parity, one stop bit
	outb(port + FIFO_CTRL_REG, 0x7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(port + MODEM_CTRL_REG, 0x0B);    // IRQs enabled, RTS/DSR set
	outb(port + INTERRUPT_EN_REG, 0x0F);	// 
}

void serial_write_str(uint16_t port, const char* str)
{
	while (*str)
	{
		serial_write(port, *str);
		str++;
	}
}

void serial_init()
{

}

void serial_printf(uint16_t port, const char* format, ...)
{
	char buff[128];
	va_list args;
	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);
	serial_write_str(port, buff);
}

void serial_read(uint16_t port, serial_callback_t handler)
{
	init_port(SERIAL_PORT_COM1);
	com1_handler = handler;
}