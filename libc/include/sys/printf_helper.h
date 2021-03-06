#pragma once

#include <sys/cdefs.h>

__LIBC_BEGIN_H

#include <stddef.h>
#include <stdarg.h>

int printf_helper(char* buff, size_t buff_sz, const char* format, va_list args);

__LIBC_END_H