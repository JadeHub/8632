#include "task.h"

#include <kernel/memory/kmalloc.h>
#include <kernel/memory/paging.h>
#include <kernel/x86/interrupts.h>
#include <kernel/x86/gdt.h>
#include <kernel/utils.h>
#include <drivers/console.h>

extern uint32_t read_eip();
extern void perform_task_switch(uint32_t eip, uint32_t ebp, uint32_t esp, uint32_t dir_addr);

extern heap_t* kheap;

//extern void move_stack(void *new_stack_start, uint32_t size, uint32_t initial_esp);

#define KERNEL_STACK_SIZE 2048

process_t* k_proc = 0; //The kernal execution context
thread_t* current_thread = 0;
uint32_t next_pid = 0;

process_t* test_p = 0;

uint8_t tt = 0;

void switch_task();

void on_timer(isr_state_t* regs)
{
	//con_printf("Timer\n");
	switch_task();
}

void switch_task()
{
	if(tt < 2 /*|| !current_thread*/)
		return;
	//tt=0;
	process_t* next_proc = 0;
	uint32_t dummy;
	uint32_t* esp_ptr;

	//uint32_t esp, ebp, eip;
	//asm volatile("mov %%esp, %0" : "=r"(esp));
	//asm volatile("mov %%ebp, %0" : "=r"(ebp));
	//eip = read_eip();
	//if (eip == 0x12345)
	//	return;

	if(current_thread)
	{
	//	current_thread->eip = eip;
		//current_thread->ebp = ebp;
		//current_thread->esp = esp;
		esp_ptr = &current_thread->esp;
		next_proc = current_thread->process->next;
	}
	if(next_proc == 0)
	{
		next_proc = test_p;
		esp_ptr = &dummy;
	}

	current_thread = next_proc->main_thread;

	//eip = current_thread->eip;
	//ebp = current_thread->ebp;
	//esp = current_thread->esp;
	
	//con_printf("Switching to %x eip %x ebp %x esp %x\n", next_proc->id, current_thread->eip, current_thread->ebp, current_thread->esp);
	con_printf("Switching to %x\n", next_proc->id);

	current_directory = next_proc->pages;
	set_kernel_stack(current_thread->k_stack + KERNEL_STACK_SIZE);
	//bochs_dbg();

	perform_task_switch(next_proc->main_thread->eip,
			esp_ptr,
			next_proc->main_thread->esp,
			next_proc->pages->physicalAddr);

	//bochs_dbg();
	//task_switch2()
}

void idle_task()
{
	for(;;)
		con_write("Going idle\n");
}

void task_new_proc(uint8_t* code, uint32_t len)
{
	//disable_interrupts();

	process_t* p = (process_t*)kmalloc(sizeof(process_t));
	p->id = next_pid++;
	p->next = 0;

	p->pages = clone_directory(kernel_directory);

	switch_page_directory(p->pages);
	//code at 0x00500000
	uint32_t start = 0x00500000;
	uint32_t end = start + 0x3000;
	alloc_pages(p->pages, start, end);
	uint32_t entry = start;
	memcpy((uint8_t*)start, code, len);

	*((uint8_t*)start+0x400) = (uint8_t)p->id;
	
	//heap at 0x00700000
	start = 0x00700000;
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
	uint32_t addr = p->main_thread->k_stack + KERNEL_STACK_SIZE - 4;
	con_printf("addr = %x\n", addr);
	*(uint32_t*)(p->main_thread->k_stack + KERNEL_STACK_SIZE - 4) = entry;
//	if (p->id == 1)
	//	*(uint32_t*)(p->main_thread->k_stack + KERNEL_STACK_SIZE - 4) = &idle_task;

	*(uint32_t*)(p->main_thread->k_stack + KERNEL_STACK_SIZE - 8) = 1; //ebx
	*(uint32_t*)(p->main_thread->k_stack + KERNEL_STACK_SIZE - 12) = 2; //esi
	*(uint32_t*)(p->main_thread->k_stack + KERNEL_STACK_SIZE - 16) = 3; //edi
	*(uint32_t*)(p->main_thread->k_stack + KERNEL_STACK_SIZE - 20) = p->main_thread->k_stack; //ebp

	con_printf("Allocated stack at %x %x\n", p->main_thread->stack_top, p->main_thread->k_stack);

	p->main_thread->esp = p->main_thread->ebp = p->main_thread->k_stack + KERNEL_STACK_SIZE - 20;
	p->main_thread->eip = entry;


	//bochs_dbg();
	/*perform_task_switch(p->main_thread->eip,
		p->main_thread->ebp,
		p->main_thread->esp,
		p->pages->physicalAddr);*/

//	process_t* tmp = k_proc;
	//while(tmp->next)
		//tmp = tmp->next;
	//tmp->next = p;
	/*
	if (!current_thread)
		current_thread = p->main_thread;
	else
		current_thread->process->next = p;
*/
	if(test_p)
	{
		test_p->next = p;
		p->next = test_p;

	}
	else
	{
		test_p = p;
	}
	
	switch_page_directory(kernel_directory);
	tt++;
	//enable_interrupts();
}

void task_init(page_directory_t* kernel_pages, uint32_t initial_esp)
{
   // disable_interrupts();

    // Relocate the stack so we know where it is.
    //move_stack((void*)0xE0000000, 0x2000, initial_esp);
    set_kernel_stack(initial_esp);

	k_proc = (process_t*)kmalloc(sizeof(process_t));
	k_proc->id = next_pid++;
	k_proc->pages = kernel_pages;
	k_proc->heap = kheap;
	k_proc->next = 0;
	k_proc->main_thread = (thread_t*)kmalloc(sizeof(thread_t));
	k_proc->main_thread->process = k_proc;
	k_proc->main_thread->stack_top = (uint32_t)alloc(0x1000, 1, kheap) + 0x1000;

	k_proc->main_thread->ebp = k_proc->main_thread->stack_top;
	k_proc->main_thread->esp = k_proc->main_thread->stack_top;
	k_proc->main_thread->eip = &idle_task;
	k_proc->main_thread->k_stack = kmalloc_a(KERNEL_STACK_SIZE);
	k_proc->main_thread->next = 0;
	//current_thread = k_proc->main_thread;
  //  enable_interrupts();
}

