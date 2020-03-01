#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

void con_init();
void con_write_buff(const char* buff, size_t sz);
void con_set_cur(uint8_t col, uint8_t row);
void con_putc(char);
