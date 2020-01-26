#pragma once

#include <stdint.h>

#include <kernel/elf32/elf32.h>

void dbg_init(const elf_image_t* kernel_image);

const elf_image_t* dbg_kernel_image();

typedef void (*dbg_stack_callback_t)(const char* fn, uint32_t addr, uint32_t sz, uint32_t ebp, uint32_t ip);

uint32_t dbg_unwind_stack(const elf_image_t* image, uint32_t ebp, dbg_stack_callback_t cb);

elf_fn_symbol_t* dbg_find_function(const elf_image_t* image, uint32_t address);

void dbg_dump_stack();