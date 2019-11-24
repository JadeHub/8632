#pragma once

#include <stdint.h>

void con_init();
void con_write(char *str);
void con_set_cur(uint8_t col, uint8_t row);
void con_write_hex(uint32_t n);
