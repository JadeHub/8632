#pragma once

#include <stdint.h>

typedef void (*timer_callback_t)(uint64_t);

void timer_init(uint32_t, timer_callback_t);
