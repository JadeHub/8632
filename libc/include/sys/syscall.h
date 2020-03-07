#pragma once

#include <stdint.h>

#include <stddef.h>

void* sys_alloc(size_t);
void sys_print_str(const char*, uint32_t);
void sys_exit(uint32_t);
uint32_t sys_open(const char*, uint32_t);
size_t sys_read(uint32_t, uint8_t*, size_t);
void sys_close(uint32_t);
void sys_sleep_ms(uint32_t);