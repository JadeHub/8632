#pragma once

#include <stdint.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

uint16_t inw(uint16_t port);

void insl(unsigned short int __port, void *__addr, unsigned long int __count);