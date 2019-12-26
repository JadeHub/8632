#pragma once

#include <sys/cdefs.h>

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

__LIBC_END_H