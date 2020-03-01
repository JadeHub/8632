#include "elf32.h"
#include <kernel/memory/kmalloc.h>
#include <kernel/fault.h>
#include <kernel/utils.h>
#include <drivers/console.h>
#include <kernel/memory/paging.h>

#include <stddef.h>
#include <stdio.h>

static inline const char* _get_tbl_name(const elf_section_t* tbl, uint32_t name)
{
	ASSERT(name < tbl->sz);
	return (const char*)(tbl->data + name);
}

static void _add_fn_symbol(elf_image_t* image, elf_sym_t* symbol)
{
	elf_fn_symbol_t* add = (elf_fn_symbol_t*)kmalloc(sizeof(elf_fn_symbol_t));
	memset(add, 0, sizeof(elf_fn_symbol_t));
	add->name = (const char*)(image->symbol_strs.data + symbol->name);
	add->address = symbol->value;
	add->section_idx = symbol->shndx;
	add->type = symbol->info;
	add->size = symbol->size;
	add->next = add->prev = 0;
	//printf("Adding %s at 0x%08x\n", add->name, add->address);

	if (!image->fn_sym_list)
	{
		//first item
		image->fn_sym_list = add;
	}
	else if (add->address < image->fn_sym_list->address)
	{
		//insert at start
		add->next = image->fn_sym_list;
		image->fn_sym_list->prev = add;
		image->fn_sym_list = add;
	}
	else
	{
		//sorted insert
		elf_fn_symbol_t* fn = image->fn_sym_list;

		while (fn)
		{
			if (add->address < fn->address)
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
		}
	}
}

static bool _is_valid_header(const elf_hdr_t* hdr)
{
	return hdr->ehsize >= sizeof(elf_hdr_t) &&
		hdr->ident[0] == 0x7F &&
		hdr->ident[1] == 'E' &&
		hdr->ident[2] == 'L' &&
		hdr->ident[3] == 'F';
}

static void _load_elf_data(page_directory_t* pages, const uint8_t* src, elf_phdr_t* phdr)
{
	alloc_pages(pages, phdr->vaddr, phdr->vaddr + phdr->memsz);
	memcpy((void*)phdr->vaddr, src + phdr->offset, phdr->filesz);
	if (phdr->memsz > phdr->filesz)
		memset((void*)(phdr->vaddr + phdr->filesz), 0, phdr->memsz - phdr->filesz);
	
}

uint32_t elf_load_raw_image(page_directory_t* pages, const char* name, const uint8_t* data, uint32_t sz)
{
	ASSERT(sz >= sizeof(elf_hdr_t));
	elf_hdr_t* hdr = (elf_hdr_t*)data;

	if (!_is_valid_header(hdr) || hdr->type != ELF_TYPE_EXE || hdr->machine != ELF_MACH_I386)
	{
		KLOG(LL_ERR, "ELF", "failed to load Elf image %s\n", name);
		printf("invalid %d %d %d %d\n", hdr->ident[0], hdr->ident[1], hdr->ident[2], hdr->ident[3]);
		return 0;
	}

	elf_phdr_t* phdr;
	for (int i = 0; i < hdr->phnum; i++)
	{
		phdr = (elf_phdr_t*)(data + hdr->phoff + (i * hdr->phentsize));

		phdr->memsz = 0x8000;
		_load_elf_data(pages, data, phdr);
		printf("PHeader type 0x%x offset 0x%08x vaddr %08x mem sz %08x\n", phdr->type, phdr->offset, phdr->vaddr, phdr->memsz);
		printf("Section header 0x%x hdr 0x%x\n", hdr->shoff, hdr);
	}

	//elf_shdr_t* section = (elf_shdr_t*)((data + hdr->shoff) + hdr->shstrndx * hdr->shentsize);
	//printf("josh %s\n", data+section->offset+section->name);
	elf_image_t* elf_img = elf_load_symbol_data(name, data,
			data+hdr->shoff, hdr->shnum, hdr->shentsize, hdr->shstrndx);

	return hdr->entry;
_err_ret:
	KLOG(LL_ERR, "ELF", "error loading Elf image %s\n", name);
	return 0;
}

elf_image_t* elf_load_symbol_data(const char* name, const uint8_t* base_address,
		const uint8_t* sections, uint32_t section_count, uint32_t section_size, uint32_t section_name_idx)
{
	elf_image_t* elf = (elf_image_t*)kmalloc(sizeof(elf_image_t));
	memset(elf, 0, sizeof(elf_image_t));

	//section name table section
	elf_shdr_t* section = (elf_shdr_t*)(sections + section_name_idx * section_size);
	elf->section_strs.data = (char*)kmalloc(section->size);
	memcpy((uint8_t*)elf->section_strs.data, base_address + section->offset, section->size);
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
			memcpy((uint8_t*)elf->symbol_strs.data, base_address + section->offset, section->size);
			elf->symbol_strs.sz = section->size;
			elf->symbol_strs.name = _get_tbl_name(&elf->section_strs, section->name);
			elf->symbol_strs.address = section->addr;
			break;
		}
	}
	if (!elf->symbol_strs.data)
	{
		KLOG(LL_ERR, "ELF", "failed to find .strtab section in %s\n", name);
		printf("aaa\n");
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

	return elf;

_err_ret:
	kfree(elf->section_strs.data);
	kfree(elf->symbol_strs.data);
	kfree(elf);
	return NULL;
}
