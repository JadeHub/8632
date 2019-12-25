#pragma once

#include <sys/cdefs.h>

#include <stddef.h>

__LIBC_BEGIN_H

char* strcpy(char* dest, const char* source);
char* strchr(const char* str, int ch);
size_t strlen(const char* buff);
int strcmp(const char* lhs, const char* rhs);
void* memset(void* dest, int ch, size_t count);
void* memcpy(void* dest, const void* src, size_t count);

__LIBC_END_H