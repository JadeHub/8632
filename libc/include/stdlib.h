#pragma once

#include <sys/cdefs.h>

#include <stddef.h>

__LIBC_BEGIN_H

int atoi(const char*);

void abort();
void* malloc(size_t sz);
void free(void* ptr);
int atexit(void (*)(void) );
char* getenv(const char* p);

__LIBC_END_H