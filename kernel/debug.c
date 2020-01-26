#include "debug.h"

#include <kernel/utils.h>
#include <kernel/tasks/sched.h>
#include <kernel/x86/interrupts.h>
#include <drivers/console.h>

#include <stddef.h>

static const elf_image_t* _k_image = NULL;

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

elf_fn_symbol_t* dbg_find_function(const elf_image_t* image, uint32_t address)
{
	return _find_fn_containing(image, address);
}

void dbg_unwind_stack2(const elf_image_t* image, uint32_t eip, uint32_t ebp, dbg_stack_callback_t cb)
{
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
				con_printf("isr_state at %x ebp %x eip %x\n", istate, istate->ebp, istate->eip);
				ebp = istate->ebp;
				eip = istate->eip;
				image = sched_cur_proc()->elf_img;
				continue;
			}
		}
		else
		{
			(*cb)("Unknown function", eip, 0, ebp, eip);
		}

		eip = *(uint32_t*)(ebp + 4);
		ebp = *(uint32_t*)(ebp);
	}
}

uint32_t dbg_unwind_stack(const elf_image_t* image, uint32_t ebp, dbg_stack_callback_t cb)
{
	con_printf("Unwinding %08x %x\n", ebp, image);
	
	uint32_t rtn_addr;
	elf_fn_symbol_t* fn;
	int count = 0;
	while (ebp != 0)
	{
		rtn_addr = *(uint32_t*)(ebp + 4);
		if (rtn_addr == 0)
			break;
		fn = _find_fn_containing(image, rtn_addr);
	
		if (fn)
		{
			(*cb)(fn->name, fn->address, fn->size, ebp, rtn_addr);
			//con_printf("Stack %s addr: %08x ebp: %08x\n", fn->name, rtn_addr, ebp);

			if (strcmp(fn->name, "isr_common_stub") == 0)
			{
				isr_state_t* istate = (isr_state_t*)(ebp + 12);
				con_printf("isr_state at %x ebp %x\n", istate, istate->ebp);
				ebp = istate->ebp;
				image = sched_cur_proc()->elf_img;

			/*	fn = _find_fn_containing(image, istate->eip);
				if (fn)
				{
					(*cb)(fn->name, fn->address, fn->size, ebp, rtn_addr);
				}
				else
				{
					con_printf("didnt find %x\n", istate->eip);
				}*/
				continue;
			}
		}
		else
		{
			
			(*cb)("Unknown function", rtn_addr, 0, ebp, rtn_addr);
		}
		ebp = *(uint32_t*)(ebp);
		count++;
		//if (count > 20) break;
	}
	return 0;
}

static void _stack_unwind_cb(const char* name, uint32_t addr, uint32_t sz, uint32_t ebp, uint32_t ip)
{
	con_printf("%08x %-20s at: %08x to %08x ebp: %08x\n", ip, name, addr, addr + sz, ebp);
}

extern uint32_t regs_ebp();

extern uint32_t regs_eip();

void dbg_dump_stack()
{
	uint32_t ebp = regs_ebp();
	uint32_t eip = regs_eip();

	dbg_unwind_stack2(dbg_kernel_image(), eip, ebp, &_stack_unwind_cb);
	bochs_dbg();
}