#pragma once

#include <stdint.h>

#define SERIAL_PORT_COM1 0x3F8

void serial_init();

void serial_printf(uint16_t port, const char* format, ...);