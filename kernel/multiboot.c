#include "multiboot.h"

#include <kernel/utils.h>
#include <kernel/fault.h>
#include <drivers/console.h>

#include <stdio.h>
#include <stddef.h>

static elf_image_t* _kernel_elf = NULL;
static const multiboot_data_t* _mb_data = NULL;

static elf_image_t* _load_kernel_image()
{
	//the multiboot data has set the addr value of the sections to the locations they were
	//loaded to. In order to share code with the elf file loading we set the offsets to the address
	//and load with a base address of 0
	const uint8_t* sections = (const uint8_t*)_mb_data->elf_sections.address;
	for (int i = 0; i < _mb_data->elf_sections.count; i++)
	{
		elf_shdr_t* tmp = (elf_shdr_t*)(sections + (i * _mb_data->elf_sections.size));
		tmp->offset = tmp->addr;
	}
	return elf_load_symbol_data("kernel",
		0,
		(uint8_t*)_mb_data->elf_sections.address,
		_mb_data->elf_sections.count,
		_mb_data->elf_sections.size,
		_mb_data->elf_sections.shndx);
}

void mb_init(const multiboot_data_t* data)
{
	ASSERT(!_mb_data);
	_mb_data = data;

	for(int i=0; i<data->mod_count;i++)
	{
		module_data_t* mod = &data->modules[i];
		KLOG(LL_INFO, "BOOT", "Loaded multiboot module %s 0x%08x 0x%08x 0x%x\n",
				data->modules->name, data->modules->start, data->modules->end, data->modules->end - data->modules->start);
	}
}

elf_image_t* mb_get_kernel_elf()
{
	if (!_kernel_elf)
		_kernel_elf = _load_kernel_image();
	return _kernel_elf;
	
}

module_data_t* mb_find_module(const char* name)
{
	const multiboot_data_t* data = _mb_data;
	for (int i = 0; i < data->mod_count; i++)
	{
		module_data_t* mod = &data->modules[i];
		if(strcmp(mod->name, name) == 0)
		{
			//printf("Module %s at %08x\n", mod->name, mod->start);
			return mod;
		}
	}
	return NULL;
}

uint32_t mb_copy_mod(const char* name, uint8_t* buff, uint32_t buff_len)
{
	module_data_t* mod = mb_find_module(name);
	if (!mod)
	{
		printf("Module %s not found\n", name);
		return 0;
	}

	uint32_t len = mod->end - mod->start;
	if (len > buff_len)
	{
		printf("Module length too long %u > %u\n", len, buff_len);
		return 0;
	}
	memcpy(buff, (void*)mod->start, len);
	return len;
}