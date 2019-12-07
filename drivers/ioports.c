#include "ioports.h"

void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (val));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
	asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}


uint16_t inw(uint16_t port)
{
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

void insl(unsigned short int port, void *addr, unsigned long int count)
{
	asm volatile("cld ; rep ; insl":"=D" (addr), "=c" (count)
		: "d" (port), "0" (addr), "1" (count));
}
