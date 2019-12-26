#pragma once

#include <sys/cdefs.h>

__LIBC_BEGIN_H

int execv(const char*, char* const []);
int execve(const char*, char* const [], char* const []);
int execvp(const char*, char* const []);
pid_t fork(void);

__LIBC_END_H
