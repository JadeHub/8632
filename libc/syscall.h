#pragma once

#include <stdint.h>

extern void perform_syscall(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5);

#define SYSCALL_MALLOC 1
#define SYSCALL_PRINT 2
#define SYSCALL_EXIT 3

#define SYSCALL1(code, param) perform_syscall(code, param, 0, 0, 0, 0)