#pragma once

#include <kernel/memory/paging.h>

#include <stdint.h>

//http://www.skyfree.org/linux/references/ELF_Format.pdf
//Section header
typedef struct elf_shdr
{
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t addr;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t addralign;
	uint32_t entsize;
} elf_shdr_t;

//program header
typedef struct elf_phdr
{
	uint32_t type;
	uint32_t offset;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t filesz;
	uint32_t memsz;
	uint32_t flags;
	uint32_t align;
}elf_phdr_t;

//Symbol table entry
typedef struct elf_sym
{
	uint32_t name;
	uint32_t value;
	uint32_t size;
	uint8_t info;
	uint8_t other;
	uint16_t shndx; //index of section this symbol relates to 
} elf_sym_t;

#define ELF_TYPE_NONE 0
#define ELF_TYPE_RELOC 1
#define ELF_TYPE_EXE 2
#define ELF_TYPE_DYN 3
#define ELF_TYPE_CORE 4

#define ELF_MACH_I386 3

#define ELF_SECTION_WRITE 0x01	//section data should be writable
#define ELF_SECTION_ALLOC 0x02	//section occupies memory during execution
#define ELF_SECTION_EXECUTABLE 4	//section contains executable instructions

typedef struct elf_hdr
{
	unsigned char ident[16];
	uint16_t      type;
	uint16_t      machine;
	uint32_t      version;
	uint32_t      entry;
	uint32_t      phoff;
	uint32_t      shoff;
	uint32_t      flags;
	uint16_t      ehsize;
	uint16_t      phentsize;
	uint16_t      phnum;
	uint16_t      shentsize;
	uint16_t      shnum;
	uint16_t      shstrndx;
} elf_hdr_t;

typedef struct fn_symbol
{
	const char* name;
	uint32_t address;
	uint32_t size;
	uint32_t type;
	uint32_t section_idx;
	struct fn_symbol* next;
	struct fn_symbol* prev;
}elf_fn_symbol_t;

typedef struct elf_section
{
	const char* name;
	uint8_t* data;
	uint32_t sz;
	uint32_t address;
}elf_section_t;

typedef struct elf_image
{
	elf_fn_symbol_t* fn_sym_list;
	elf_section_t section_strs;
	elf_section_t symbol_strs;
}elf_image_t;

elf_image_t* elf_load_symbol_data(const char* name, const uint8_t* base_address,
	const uint8_t* sections, uint32_t section_count, uint32_t section_size, uint32_t section_name_idx);

uint32_t elf_load_raw_image(page_directory_t* pages, const char* name, const uint8_t* data, uint32_t sz);