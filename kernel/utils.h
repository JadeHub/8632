#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

void* memset(void* address, uint8_t val, uint32_t len);
void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len);

static inline void bochs_dbg()
{
	asm volatile("xchg %bx, %bx");
}

void printf_helper(void (*emit)(char), const char* fmt, va_list args);

void sprintf(char* buff, const char* format, ...);
void vsprintf(char* buff, const char* format, va_list args);
uint32_t strlen(const char* buff);
int strcmp(const char* lhs, const char* rhs);
char* strcpy(char* dest, const char* source);
int atoi(const char*);
int isspace(int c);
static inline bool is_digit(uint8_t c)
{
	return (c >= '0') && (c <= '9');
}