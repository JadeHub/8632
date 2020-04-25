#pragma once

#include <stdint.h>

#include <kernel/elf32/elf32.h>
#include <kernel/utils.h>

void dbg_init(const elf_image_t* kernel_image);

const elf_image_t* dbg_kernel_image();

void dbg_dump_current_stack();

void dbg_dump_stack(const elf_image_t*, uint32_t ebp, uint32_t eip);

void dbg_break();