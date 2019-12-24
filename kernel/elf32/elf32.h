#pragma once

#include <stdint.h>

typedef struct fn_symbol
{
	const char* name;
	uint32_t address;
	uint32_t size;
	uint32_t type;
	uint32_t section_idx;
	struct fn_symbol* next;
	struct fn_symbol* prev;
}fn_symbol_t;

typedef struct elf32_str_tbl
{
	const char* data;
	uint32_t len;
}elf32_str_tbl_t;

typedef struct elf32_image
{
	fn_symbol_t* fn_sym_list;
	elf32_str_tbl_t section_strs;
	elf32_str_tbl_t symbol_strs;
}elf32_image_t;

elf32_image_t* elf_load_image(uint8_t* sections, uint32_t section_count, uint32_t section_size, uint32_t section_name_idx);