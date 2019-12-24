#include "elf32.h"
#include <kernel/memory/kmalloc.h>
#include <kernel/fault.h>
#include <kernel/utils.h>
#include <drivers/console.h>

#include <stddef.h>

//http://www.skyfree.org/linux/references/ELF_Format.pdf
//Section header
typedef struct elf32_shdr
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
} elf32_shdr_t;

//Symbol table entry
typedef struct elf32_sym
{
	uint32_t name;
	uint32_t value;
	uint32_t size;
	uint8_t info;
	uint8_t other;
	uint16_t shndx; //index of section this symbol relates to 
} elf32_sym_t;

static inline const char* _get_tbl_name(const elf32_str_tbl_t* tbl, uint32_t name)
{
	ASSERT(name < tbl->len);
	return (const char*)(tbl->data + name);
}

static void _add_fn_symbol(elf32_image_t* image, elf32_sym_t* symbol)
{
	fn_symbol_t* fn = image->fn_sym_list;
	fn_symbol_t* add = (fn_symbol_t*)kmalloc(sizeof(fn_symbol_t));
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

elf32_image_t* elf_load_image(uint8_t* sections, uint32_t section_count, uint32_t section_size, uint32_t section_name_idx)
{
	elf32_image_t* elf = (elf32_image_t*)kmalloc(sizeof(elf32_image_t));
	memset(elf, 0, sizeof(elf32_image_t));

	//section name table section, identified by shndx in boot data
	elf32_shdr_t* section = (elf32_shdr_t*)(sections + section_name_idx * section_size);
	elf->section_strs.data = (char*)kmalloc(section->size);
	memcpy((uint8_t*)elf->section_strs.data, (const uint8_t*)section->addr, section->size);
	elf->section_strs.len = section->size;

	//find symbol name table section
	for (int i = 0; i < section_count; i++)
	{
		section = (elf32_shdr_t*)(sections + (i * section_size));
		if (section->type == 3 && streq(_get_tbl_name(&elf->section_strs, section->name), ".strtab"))
		{
			elf->symbol_strs.data = (char*)kmalloc(section->size);
			memcpy((uint8_t*)elf->symbol_strs.data, (const uint8_t*)section->addr, section->size);
			elf->symbol_strs.len = section->size;
			break;
		}
	}
	if (!elf->symbol_strs.data)
	{
		//klog
		goto _err_ret;
	}

	//find symbol section
	section = NULL;
	for (int i = 0; i < section_count; i++)
	{
		elf32_shdr_t* tmp = (elf32_shdr_t*)(sections + (i * section_size));
		if (tmp->type == 2)
		{
			//con_printf("SECTION %d add %d size %d\n", i, tmp->addr, tmp->size);
			section = tmp;
			break;
		}
	}
	if (!section)
	{
		goto _err_ret;
	}
	uint32_t count = 0;
	//Find the function symbols and add them
	//con_printf("SECTION type %d size %d\n", section->type, section->size);

	for (int i = 0; i < section->size; i += sizeof(elf32_sym_t))
	{
		elf32_sym_t* symbol = (elf32_sym_t*)(section->addr + i);
		if (symbol->name && (symbol->info & 2))
		{
			//Function with a name
			_add_fn_symbol(elf, symbol);
			count++;
		}
	}
	//con_printf("ELF size %d %d symbols\n", section->size, count);

	return elf;

_err_ret:
	con_printf("Elf Error\n");
	if (elf)
	{
		kfree(elf);
		elf = NULL;
	}
	return NULL;
}
