#pragma once

#include <stdint.h>

typedef void (*timer_callback_t)();

void timer_init(uint32_t);
void timer_add_callback(timer_callback_t);