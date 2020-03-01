#pragma once

#include <sys/cdefs.h>

#include <stddef.h>
#include <stdarg.h>

typedef void (*flush_fn_t)(const char* buff, int* buff_pos, size_t buff_sz);
int printf_helper(char* buff, size_t buff_sz, flush_fn_t flush, const char* format, va_list args);
