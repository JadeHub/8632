#include "elf32.h"
#include <kernel/memory/kmalloc.h>
#include <kernel/fault.h>
#include <kernel/utils.h>
#include <drivers/console.h>

#include <stddef.h>

static inline const char* _get_tbl_name(const elf_section_t* tbl, uint32_t name)
{
	ASSERT(name < tbl->sz);
	return (const char*)(tbl->data + name);
}

static void _add_fn_symbol(elf_image_t* image, elf_sym_t* symbol)
{
	elf_fn_symbol_t* fn = image->fn_sym_list;
	elf_fn_symbol_t* add = (elf_fn_symbol_t*)kmalloc(sizeof(elf_fn_symbol_t));
	add->name = (const char*)(image->symbol_strs.data + symbol->name);
	add->address = symbol->value;
	add->section_idx = symbol->shndx;
	add->type = symbol->info;
	add->size = symbol->size;
	add->next = add->prev = 0;
	//con_printf("Adding %s at %08x\n", add->name, add->address);
	do
	{
		if (!fn)
		{
			//first
			image->fn_sym_list = add;
			break;
		}
		if (fn->address > add->address)
		{
			//sorted insert
			add->next = fn;
			if (fn->prev)
				fn->prev->next = add;
			add->prev = fn->prev;
			fn->prev = add;
			break;
		}
		if (fn->next == 0)
		{
			//last
			add->prev = fn;
			fn->next = add;
			break;
		}
		fn = fn->next;
	} while (fn);
}

static bool _is_valid_header(const elf_hdr_t* hdr)
{
	return hdr->ehsize >= sizeof(elf_hdr_t) &&
		hdr->ident[0] == 0x7F &&
		hdr->ident[1] == 'E' &&
		hdr->ident[2] == 'L' &&
		hdr->ident[3] == 'F';
}

elf_image_t* elf_load_raw_image(const char* name, const uint8_t* data, uint32_t sz)
{
	ASSERT(sz >= sizeof(elf_hdr_t));
	elf_hdr_t* hdr = (elf_hdr_t*)data;

	if (!_is_valid_header(hdr) || hdr->type != ELF_TYPE_EXE || hdr->machine != ELF_MACH_I386)
	{
		KLOG(LL_ERR, "ELF", "failed to load Elf image %s\n", name);
		con_printf("invalid %d %d %d %d\n", hdr->ident[0], hdr->ident[1], hdr->ident[2], hdr->ident[3]);
		return NULL;
	}

	elf_image_t* elf = (elf_image_t*)kmalloc(sizeof(elf_image_t));
	memset(elf, 0, sizeof(elf_image_t));
	
	elf = elf_load_section_data(name, (uint32_t)data, (uint8_t*)(data + hdr->shoff), hdr->shnum, hdr->shentsize, hdr->shstrndx);

	elf_phdr_t* phdr;

	for (int i = 0; i < hdr->phnum; i++)
	{
		phdr = (elf_phdr_t*)(data + hdr->phoff + (i * hdr->phentsize));

		con_printf("PHeader type %x offset %08x vaddr %08x mem sz %08x\n", phdr->type, phdr->offset, phdr->vaddr, phdr->memsz);
	}

	return elf;
_err_ret:
	KLOG(LL_ERR, "ELF", "error loading Elf image %s\n", name);
	kfree(elf);
	return NULL;
}

elf_image_t* elf_load_section_data(const char* name, uint32_t base_address,
		uint8_t* sections, uint32_t section_count, uint32_t section_size, uint32_t section_name_idx)
{
	elf_image_t* elf = (elf_image_t*)kmalloc(sizeof(elf_image_t));
	memset(elf, 0, sizeof(elf_image_t));

	//section name table section
	elf_shdr_t* section = (elf_shdr_t*)(sections + section_name_idx * section_size);
	elf->section_strs.data = (char*)kmalloc(section->size);
	memcpy((uint8_t*)elf->section_strs.data, (const uint8_t*)base_address + section->offset, section->size);
	elf->section_strs.sz = section->size;
	elf->section_strs.name = _get_tbl_name(&elf->section_strs, section->name);
	elf->section_strs.address = section->addr;
	//find symbol name table section
	for (int i = 0; i < section_count; i++)
	{
		section = (elf_shdr_t*)(sections + (i * section_size));

		if (section->type == 3 && streq(_get_tbl_name(&elf->section_strs, section->name), ".strtab"))
		{
			elf->symbol_strs.data = (char*)kmalloc(section->size);
			memcpy((uint8_t*)elf->symbol_strs.data, (const uint8_t*)base_address + section->offset, section->size);
			elf->symbol_strs.sz = section->size;
			elf->symbol_strs.name = _get_tbl_name(&elf->section_strs, section->name);
			elf->symbol_strs.address = section->addr;
			break;
		}
	}
	if (!elf->symbol_strs.data)
	{
		KLOG(LL_ERR, "ELF", "failed to find .strtab section in %s\n", name);
		con_printf("aaa\n");
		goto _err_ret;
	}

	//find symbol section
	section = NULL;
	for (int i = 0; i < section_count; i++)
	{
		elf_shdr_t* tmp = (elf_shdr_t*)(sections + (i * section_size));
		if (tmp->type == 2)
		{
			section = tmp;
			break;
		}
	}
	if (section)
	{		
		//Find the function symbols and add them
		for (int i = 0; i < section->size; i += sizeof(elf_sym_t))
		{
			elf_sym_t* symbol = (elf_sym_t*)(base_address + section->offset + i);
			if (symbol->name && (symbol->info & 2))
			{
				//Function with a name
				_add_fn_symbol(elf, symbol);
			}
		}
	}

	//find executable section
	section = NULL;
	for (int i = 0; i < section_count; i++)
	{
		elf_shdr_t* tmp = (elf_shdr_t*)(sections + (i * section_size));


		if (tmp->flags & ELF_SECTION_EXECUTABLE)
		{
			section = tmp;
			break;
		}
	}
	if (section)
	{
		elf->exec_section.sz = section->size;
		elf->exec_section.data = (uint8_t *)kmalloc(section->size);
		memcpy(elf->exec_section.data, (void*)base_address + section->offset, section->size);
		elf->exec_section.name = _get_tbl_name(&elf->section_strs, section->name);
		elf->exec_section.address = section->addr;
		con_printf("Found exe in %s section %s\n", name, elf->exec_section.name);
	}

	//.rodata
	return elf;

_err_ret:
	kfree(elf->section_strs.data);
	kfree(elf->symbol_strs.data);
	kfree(elf->exec_section.data);
	kfree(elf);
	return NULL;
}
