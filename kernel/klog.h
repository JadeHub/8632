#pragma once

#include <stdint.h>

#define LOG_MAX_LINE 256

void klog_init();
void klog_init_persistence();

typedef enum
{
	LL_INFO,
	LL_ERR
}KLOG_LEVEL;

void klog_impl(const char* file, uint32_t line, KLOG_LEVEL level, const char* module, const char* msg, ...);

//assumes static const char* _LM
#define KLOG(level, msg, ...) { klog_impl(__FILE__, __LINE__, level, _LM, msg, __VA_ARGS__); }