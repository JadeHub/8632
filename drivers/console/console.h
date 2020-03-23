#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

void con_init();
void con_dev_init();
void con_write_buff(const uint8_t* buff, size_t sz);
void con_putc(uint8_t);
