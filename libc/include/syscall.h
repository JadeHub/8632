#pragma once

#include <stdint.h>

extern uint32_t perform_syscall(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5);

#define SYSCALL_MALLOC 1
#define SYSCALL_PRINT 2
#define SYSCALL_EXIT 3
#define SYSCALL_OPEN 4
#define SYSCALL_CLOSE 5
#define SYSCALL_READ 6

#define SYSCALL1(code, param) perform_syscall(code, (uint32_t)param, 0, 0, 0, 0)
#define SYSCALL2(code, param1, param2) perform_syscall(code, (uint32_t)param1, (uint32_t)param2, 0, 0, 0)
#define SYSCALL3(code, param1, param2, param3)	\
		perform_syscall(code, (uint32_t)param1,(uint32_t)param2, (uint32_t)param3, 0, 0)
#define SYSCALL4(code, param1, param2, param3, param4)	\
		perform_syscall(code, (uint32_t)param1, (uint32_t)param2, (uint32_t)param3, (uint32_t)param4, 0)
#define SYSCALL5(code, param1, param2, param3, param4, param5)	\
		perform_syscall(code, (uint32_t)param1, (uint32_t)param2, (uint32_t)param3, (uint32_t)param4, (uint32_t)param5)
