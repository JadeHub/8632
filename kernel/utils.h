#pragma once

#include <stdint.h>
#include <stdarg.h>

void* memset(void* address, uint8_t val, uint32_t len);
void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len);

static inline void bochs_dbg()
{
	asm volatile("xchg %bx, %bx");
}

void printf_helper(void (*emit)(char), const char* fmt, va_list args);

void sprintf(char* buff, const char* format, ...);
void vsprintf(char* buff, const char* format, va_list args);