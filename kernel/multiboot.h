#pragma once

#include <kernel/elf32/elf32.h>

#include <stdint.h>

#define MULTIBOOT_INFO_MEMORY                   0x00000001
#define MULTIBOOT_INFO_BOOTDEV                  0x00000002
#define MULTIBOOT_INFO_CMDLINE                  0x00000004
#define MULTIBOOT_INFO_MODS                     0x00000008
#define MULTIBOOT_INFO_ELF_SHDR                 0X00000020
#define MULTIBOOT_INFO_MEM_MAP                  0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO               0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200
#define MULTIBOOT_INFO_APM_TABLE                0x00000400

typedef struct elf_section_data
{
	uint32_t count;
	uint32_t size;
	uint32_t address;
	uint32_t shndx;

} elf_section_data_t;

typedef struct mem_map_data
{
	uint32_t sz;
	uint32_t address_low;
	uint32_t address_high;
	uint32_t size_low;
	uint32_t size_high;
	uint32_t type;
}mem_map_data_t;

typedef struct drive_data
{

}drive_data_t;

typedef struct module_data
{
	uint32_t start;
	uint32_t end;
	const char* name;
	uint32_t reserved;
}module_data_t;

typedef struct multiboot_data
{
	uint32_t flags;
	uint32_t mem_low;
	uint32_t mem_high;
	uint32_t boot_device;
	char* cmdline;
	uint32_t mod_count;
	module_data_t* modules;
	elf_section_data_t elf_sections;
	uint32_t mmap_size;
	mem_map_data_t* mmap;
	uint32_t drives_size;
	drive_data_t* drives;
	uint32_t config_table;
	char* bootloader_name;
	uint32_t apm_table;
}multiboot_data_t;

elf_image_t* mb_get_kernel_elf();

void mb_init(const multiboot_data_t* data);

module_data_t* mb_find_module(const char* name);

/*
Copy a named module into buff and return the length
*/
uint32_t mb_copy_mod(const char* name, uint8_t* buff, uint32_t len);
