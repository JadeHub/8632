#pragma once

#include <stdint.h>

#include <kernel/elf32/elf32.h>

void dbg_init(const elf32_image_t* kernel_image);

const elf32_image_t* dbg_kernel_image();

uint32_t dbg_unwind_stack(const elf32_image_t* image, uint32_t ebp);