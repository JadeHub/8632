#include "debug.h"

#include <kernel/utils.h>
#include <kernel/tasks/sched.h>
#include <kernel/x86/interrupts.h>
#include <kernel/utils.h>

#include <stddef.h>
#include <stdio.h>

#include <kernel/x86/regs.h>

static const elf_image_t* _k_image = NULL;

typedef void (*dbg_stack_callback_t)(const char* fn, uint32_t addr, uint32_t sz, uint32_t ebp, uint32_t ip);

void dbg_init(const elf_image_t* kernel_image)
{
	_k_image = kernel_image;
}

const elf_image_t* dbg_kernel_image()
{
	return _k_image;
}

static elf_fn_symbol_t* _find_fn_containing(const elf_image_t* image, uint32_t addr)
{
	if (addr == 0)
		return NULL;

	elf_fn_symbol_t* fn = image->fn_sym_list;
	while (fn)
	{
		if (addr < fn->address)
			return fn->prev;
		if (!fn->next)
			return fn;
		fn = fn->next;
	}	
	return fn;
}

static void _dbg_unwind_stack(const elf_image_t* image, uint32_t eip, uint32_t ebp, dbg_stack_callback_t cb)
{
	static bool unwinding = false;

	if (unwinding)
		return;
	unwinding = true;

	while (eip && ebp)
	{
		elf_fn_symbol_t* fn;
		fn = _find_fn_containing(image, eip);
		if (fn)
		{
			(*cb)(fn->name, fn->address, fn->size, ebp, eip);

			if (strcmp(fn->name, "isr_handler") == 0)
			{
				//extra frame for the isr_common_stub which doesn't have a base pointer
				fn = _find_fn_containing(image, *(uint32_t*)(ebp + 4));
				if (fn)
					(*cb)(fn->name, fn->address, fn->size, ebp, eip);

				//switch to user mode
				isr_state_t* istate = (isr_state_t*)(ebp + 12);
				printf("*** Interrupt 0x%02x ***\n", istate->int_no);
				ebp = istate->ebp;
				eip = istate->eip;
				image = sched_cur_proc()->elf_img;
				continue;
			}
			else if (strcmp(fn->name, "__entry") == 0)
			{
				break;
			}
		}
		else
		{
			(*cb)("Unknown function", eip, 0, ebp, eip);
		}
		eip = *(uint32_t*)(ebp + 4);
		ebp = *(uint32_t*)(ebp);
	}
	unwinding = false;
}

static void _stack_unwind_cb(const char* name, uint32_t addr, uint32_t sz, uint32_t ebp, uint32_t ip)
{
	printf("0x%08x %-20s at: 0x%08x to 0x%08x ebp: 0x%08x\n", ip, name, addr, addr + sz, ebp);
}

void dbg_dump_stack(const elf_image_t* image, uint32_t ebp, uint32_t eip)
{
	_dbg_unwind_stack(image, eip, ebp, &_stack_unwind_cb);
}

void dbg_dump_current_stack()
{
	uint32_t ebp = regs_ebp();
	uint32_t eip = regs_eip();

	_dbg_unwind_stack(dbg_kernel_image(), eip, ebp, &_stack_unwind_cb);
}

void dbg_break()
{
	bochs_dbg();
}