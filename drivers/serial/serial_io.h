#pragma once

#include <stdint.h>

#define SERIAL_PORT_COM1 0x3F8

void serial_init();

typedef void (*serial_callback_t)(uint16_t port, uint8_t data);

void serial_read(uint16_t port, serial_callback_t handler);

void serial_write(uint16_t port, uint8_t c);
void serial_write_str(uint16_t port, const char* str);

void serial_printf(uint16_t port, const char* format, ...);