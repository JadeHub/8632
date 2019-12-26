#include "dbg_monitor.h"

#include <kernel/utils.h>
#include <kernel/debug.h>

#include <stdlib.h>

void kernel_cmd(const char* params)
{
	if (streq(params, "fn"))
	{
		uint32_t addr = atoi(params + 3);
		//dump kernel function symbols
		const elf_fn_symbol_t* fn = dbg_find_function(dbg_kernel_image(), addr);
		if (fn)
			dbg_mon_output_line("%-30s at: %08x to %08x", fn->name, fn->address, fn->address + fn->size);
		else
			dbg_mon_output_line("No function found at %08x", addr);
	}
	else if (streq(params, "fns"))
	{
		//dump kernel function symbols
		const elf_fn_symbol_t* fn = dbg_kernel_image()->fn_sym_list;
		while (fn)
		{
			dbg_mon_output_line("%-30s at: %08x to %08x", fn->name, fn->address, fn->address + fn->size);
			fn = fn->next;
		}
	}
}