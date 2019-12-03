#include <kernel/memory/paging.h>
#include <kernel/utils.h>

#include <stdint.h>


void move_stack(void *new_stack_start, uint32_t size, uint32_t initial_esp)
{
	uint32_t i;
	// Allocate some space for the new stack.
	for (i = (uint32_t)new_stack_start;
		i >= ((uint32_t)new_stack_start - size);
		i -= 0x1000)
	{
		//		bochs_dbg();
		// General-purpose stack is in user-mode.
		alloc_frame(get_page(i, 1, current_directory), 0 /* User mode */, 1 /* Is writable */);
		//		bochs_dbg();
	}

	// Flush the TLB by reading and writing the page directory address again.
	uint32_t pd_addr;
	asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
	asm volatile("mov %0, %%cr3" : : "r" (pd_addr));

	// Old ESP and EBP, read from registers.
	uint32_t old_stack_pointer; asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
	uint32_t old_base_pointer;  asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer));

	// Offset to add to old stack addresses to get a new stack address.
	uint32_t offset = (uint32_t)new_stack_start - initial_esp;

	// New ESP and EBP.
	uint32_t new_stack_pointer = old_stack_pointer + offset;
	uint32_t new_base_pointer = old_base_pointer + offset;

	// Copy the stack.
	memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, initial_esp - old_stack_pointer);

	// Backtrace through the original stack, copying new values into
	// the new stack.  
	for (i = (uint32_t)new_stack_start; i > (uint32_t)new_stack_start - size; i -= 4)
	{
		uint32_t tmp = *(uint32_t*)i;
		// If the value of tmp is inside the range of the old stack, assume it is a base pointer
		// and remap it. This will unfortunately remap ANY value in this range, whether they are
		// base pointers or not.
		if ((old_stack_pointer < tmp) && (tmp < initial_esp))
		{
			con_write("remap!\n");
			tmp = tmp + offset;
			uint32_t *tmp2 = (uint32_t*)i;
			*tmp2 = tmp;
		}
	}

	// Change stacks.
	asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
	asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));
}