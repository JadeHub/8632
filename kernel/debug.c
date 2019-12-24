#include "debug.h"

#include <kernel/utils.h>
#include <drivers/console.h>

#include <stddef.h>

static const elf32_image_t* _k_image = NULL;

void dbg_init(const elf32_image_t* kernel_image)
{
	_k_image = kernel_image;
}

const elf32_image_t* dbg_kernel_image()
{
	return _k_image;
}

static fn_symbol_t* _find_fn_containing(const elf32_image_t* image, uint32_t addr)
{
	fn_symbol_t* fn = image->fn_sym_list;

	while (fn)
	{
		if (addr >= fn->address && addr < fn->address + fn->size)
			return fn;
		fn = fn->next;
	}

	return fn;
}

uint32_t dbg_unwind_stack(const elf32_image_t* image, uint32_t ebp)
{
	con_printf("Unwinding %08x %x\n", ebp, image);
	
	uint32_t rtn_addr;
	fn_symbol_t* fn;
	int count = 0;
	while (ebp != 0)
	{
		rtn_addr = *(uint32_t*)(ebp + 4);
		fn = _find_fn_containing(image, rtn_addr);
		if (fn)
		{
			con_printf("Stack %s addr: %08x ebp: %08x\n", fn->name, rtn_addr, ebp);


		}
		else
		{
			con_printf("Function not found addr: %08x ebp: %08x\n", rtn_addr, ebp);
		}
		ebp = *(uint32_t*)(ebp);
		count++;
		if (count > 20) break;
	}
	return 0;
}