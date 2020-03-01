#pragma once

#include <sys/cdefs.h>

#include <stddef.h>
#include <stdarg.h>

__LIBC_BEGIN_H

typedef struct _FILE {
	int unused;
}FILE;

#define SEEK_SET 0

extern FILE* stderr;
int fflush(FILE*);
int fprintf(FILE*, const char*, ...);
int fclose(FILE*);
FILE* fopen(const char*, const char*);
size_t fread(void*, size_t, size_t, FILE*);
int fseek(FILE*, long, int);
long ftell(FILE*);
size_t fwrite(const void*, size_t, size_t, FILE*);
void setbuf(FILE*, char*);

int vfprintf(FILE*, const char*, va_list);

int snprintf(char*, size_t sz, const char*, ...);
int sprintf(char*, const char*, ...);
int vsnprintf(char* buff, size_t sz, const char* format, va_list args);
int vsprintf(char* buff, const char* format, va_list args);

int vprintf(const char*, va_list);
int printf(const char*, ...);

__LIBC_END_H