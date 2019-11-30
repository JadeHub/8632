#pragma once

#include <stdint.h>

void fault_init();

void panic_impl(const char* msg, const char* file, uint32_t line);

#define KPANIC(msg) panic_impl(msg, __FILE__, __LINE__)
#define ASSERT(cond) ((cond) ?  (void)0 : panic_impl("Assert failed", __FILE__, __LINE__))