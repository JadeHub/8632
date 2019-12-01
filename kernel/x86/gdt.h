#pragma once

#include <stdint.h>

void gdt_init();
void set_kernel_stack(uint32_t stack);