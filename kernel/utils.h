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

static inline bool streq(const char* str1, const char* str2)
{
	return strcmp(str1, str2) == 0;
}

char* copy_str(const char*);