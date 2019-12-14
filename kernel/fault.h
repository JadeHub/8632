#pragma once

#include <stdint.h>

void fault_init();

void panic_impl(const char* msg, const char* file, uint32_t line);

#define KPANIC(msg) panic_impl(msg, __FILE__, __LINE__)
#define ASSERT(cond) ((cond) ?  (void)0 : panic_impl("Assert failed", __FILE__, __LINE__))

typedef enum
{
	LL_INFO,
	LL_ERR
}KLOG_LEVEL;

void klog_impl(const char* file, uint32_t line, KLOG_LEVEL level, const char* module, const char* msg, ...);

#define KLOG(level, module, msg, ...) { klog_impl(__FILE__, __LINE__, level, module, msg, __VA_ARGS__); }