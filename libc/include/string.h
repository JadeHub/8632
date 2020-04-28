#pragma once

#include <sys/cdefs.h>

#include <stddef.h>

__LIBC_BEGIN_H

char* strcpy(char* dest, const char* source);
char* strncpy(char* dest, const char* source, size_t len);
char* strchr(const char* str, int ch);
char* strrchr(const char* str, int ch);
size_t strlen(const char* buff);
int strcmp(const char* lhs, const char* rhs);
int strncmp(const char* lhs, const char* rhs, size_t count);
char* strcat(char* dest, const char* src);
void* memset(void* dest, int ch, size_t count);
void* memcpy(void* dest, const void* src, size_t count);

__LIBC_END_H