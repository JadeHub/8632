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