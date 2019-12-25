#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include <string.h>

#include <ctype.h>

static inline void bochs_dbg()
{
	asm volatile("xchg %bx, %bx");
}

void printf_helper(void (*emit)(char), const char* fmt, va_list args);

void sprintf(char* buff, const char* format, ...);
void vsprintf(char* buff, const char* format, va_list args);

static inline bool streq(const char* str1, const char* str2)
{
	return strcmp(str1, str2) == 0;
}