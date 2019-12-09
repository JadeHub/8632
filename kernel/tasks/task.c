#include "task.h"

#include <kernel/memory/kmalloc.h>
#include <kernel/memory/paging.h>
#include <kernel/x86/interrupts.h>
#include <kernel/x86/gdt.h>
#include <kernel/utils.h>
#include <drivers/console.h>

extern uint32_t read_eip();

extern void move_stack(void *new_stack_start, uint32_t size, uint32_t initial_esp);

#define KERNEL_STACK_SIZE 2048

process_t* k_proc; //The kernal execution context
thread_t* current_thread = 0;
uint32_t next_pid = 0;

process_t* test_p = 0;

uint8_t tt = 0;

void switch_task();

void on_timer(isr_state_t* regs)
{
	switch_task();
}

void switch_task()
{
	if(!tt /*|| !current_thread*/)
		return;
	tt=0;

	//con_printf("switch_task()\n");

	uint32_t esp, ebp, eip;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	asm volatile("mov %%ebp, %0" : "=r"(ebp));
	eip = read_eip();
	if (eip == 0x12345)
	{
		//con_write("\n\nswitched\n\n");
		return;
	}

	//con_printf("Saved eip %x\n", eip);

	process_t* next_proc = 0;

	if(current_thread)
	{
		current_thread->eip = eip;
		current_thread->ebp = ebp;
		current_thread->esp = esp;
		next_proc = current_thread->process->next;
	}
	if(next_proc == 0)
		next_proc = test_p;

	current_thread = next_proc->main_thread;

	eip = current_thread->eip;
	ebp = current_thread->ebp;
	esp = current_thread->esp;
	
	//con_printf("Switching to %x eip %x ebp %x esp %x\n", next_proc->id, current_thread->eip, current_thread->ebp, current_thread->esp);

	//current_directory = next_proc->pages;

	
	/*
	asm volatile("         \
      cli;                 \
	  mov %4, %%ebx; \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%ebp;       \
      mov %3, %%cr3;       \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx           "
		: : "r"(eip), "r"(esp), "r"(ebp), "r"(current_directory->physicalAddr), "r"(next_proc->id));
	//	tt=0;
	*/
	con_printf("Switching to %x\n", next_proc->id);

	//switch_page_directory(next_proc->pages);
	
	//bochs_dbg();

	
	current_directory = next_proc->pages;
	set_kernel_stack(current_thread->k_stack + KERNEL_STACK_SIZE);
	
	//bochs_dbg();
	asm volatile ("		\
		cli;			\
		mov %0, %%ecx;	\
		mov %1, %%ebp;	\
		mov %2, %%esp;	\
		mov %3, %%cr3;       \
		mov $0x12345, %%eax; \
		sti;			\
		jmp %%ecx		"
		: : "r" (next_proc->main_thread->eip),
			"r"(next_proc->main_thread->ebp),
			"r"(next_proc->main_thread->esp),
			"r"(next_proc->pages->physicalAddr)
		: "eax", "ecx");
}

void idle_task()
{
	for(;;);
}


void task_new_proc(uint8_t* code, uint32_t len)
{
	disable_interrupts();

	process_t* p = (process_t*)kmalloc(sizeof(process_t));
	p->id = next_pid++;
	p->next = 0;

	p->pages = clone_directory(kernel_directory);

	switch_page_directory(p->pages);
	//code at 0x00200000
	uint32_t start = 0x00200000;
	uint32_t end = start + 0x1000;
	alloc_pages(p->pages, start, end);
	uint32_t entry = start;
	memcpy((uint8_t*)start, code, len);

	*((uint8_t*)start+0x400) = p->id + 7;

	
	//heap at 0x00400000
	start = 0x00400000;
	end = start + 0x100000;
	alloc_pages(p->pages, start, end);
	p->heap = create_heap(p->pages, start, end, end, 0, 0);

	//thread
	p->main_thread = (thread_t*)kmalloc(sizeof(thread_t));
	p->main_thread->process = p;
	p->main_thread->next = 0;
	//stack on the heap
	p->main_thread->stack_top = (uint32_t)alloc(0x1000, 1, p->heap) + 0x1000;

	if (p->id == 1)
	{
		p->main_thread->stack_top = (uint32_t)alloc(0x1000, 1, p->heap) + 0x1000;

	}

	p->main_thread->k_stack = kmalloc(KERNEL_STACK_SIZE);

	con_printf("Allocated stack at %x %x\n", p->main_thread->stack_top, p->main_thread->k_stack);

	p->main_thread->esp = p->main_thread->ebp = p->main_thread->stack_top;
	p->main_thread->eip = entry;

	/*process_t* tmp = k_proc;
	while(tmp->next)
		tmp = tmp->next;
	tmp->next = p;*/

	
	if(test_p)
	{
		test_p->next = p;
		tt = 1;

		switch_page_directory(test_p->pages);
		con_printf("magic 0 %x %x\n", test_p->pages, *((uint8_t*)0x00200000 + 0x400));
		switch_page_directory(test_p->next->pages);
		con_printf("magic 1 %x %x\n", test_p->next->pages, *((uint8_t*)0x00200000 + 0x400));

		

	}
	else
	{
		test_p = p;
		switch_page_directory(test_p->pages);
		con_printf("magic 0 %x %x\n", test_p->pages, *((uint8_t*)0x00200000 + 0x400));

	}

	switch_page_directory(kernel_directory);
	//tt = 1;
	enable_interrupts();

}

void task_init(page_directory_t* kernel_pages, uint32_t initial_esp)
{
    disable_interrupts();

    // Relocate the stack so we know where it is.
    //move_stack((void*)0xE0000000, 0x2000, initial_esp);
    set_kernel_stack(initial_esp);

	k_proc = (process_t*)kmalloc(sizeof(process_t));
	k_proc->id = 0;
	k_proc->pages = kernel_pages;
	k_proc->next = 0;
	k_proc->main_thread = (thread_t*)kmalloc(sizeof(thread_t));
	k_proc->main_thread->process = k_proc;
	k_proc->main_thread->ebp = initial_esp;
	k_proc->main_thread->esp = initial_esp;
	k_proc->main_thread->eip = (uint32_t)&idle_task;
	k_proc->main_thread->k_stack = initial_esp - KERNEL_STACK_SIZE;
	k_proc->main_thread->next = 0;
	//current_thread = k_proc->main_thread;
    enable_interrupts();
}

