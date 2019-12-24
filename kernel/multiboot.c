#include "multiboot.h"

#include <kernel/utils.h>
#include <kernel/fault.h>
#include <drivers/console.h>

#include <stddef.h>

static elf32_image_t* _kernel_elf = NULL;
static const multiboot_data_t* _mb_data = NULL;

static uint32_t _max(uint32_t v1, uint32_t v2, uint32_t sz)
{
	//con_printf("MB Using %08x - %08x\n", v2, v2 + sz);
	return v1 > v2 + sz ? v1 : v2 + sz;
}

void mb_init(const multiboot_data_t* data)
{
	ASSERT(!_mb_data);
	_mb_data = data;
}

elf32_image_t* mb_get_kernel_elf32()
{
	if (_kernel_elf)
		return _kernel_elf;
	return elf_load_image((uint8_t*)_mb_data->elf_sections.address,
		_mb_data->elf_sections.count,
		_mb_data->elf_sections.size,
		_mb_data->elf_sections.shndx);
}