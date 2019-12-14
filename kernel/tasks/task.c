#include "task.h"

#include <kernel/memory/kmalloc.h>
#include <kernel/memory/paging.h>
#include <kernel/x86/interrupts.h>
#include <kernel/x86/gdt.h>
#include <kernel/utils.h>
#include <kernel/fault.h>
#include <drivers/console.h>

extern void start_kernel_mode_thread(uint32_t entry);
extern void start_user_mode_thread(uint32_t entry);
extern void perform_task_switch(uint32_t* esp_out, uint32_t esp, uint32_t dir_addr);

extern heap_t* kheap;

#define KERNEL_STACK_SIZE 2048

process_t* k_proc = 0; //The kernal execution context
thread_t* current_thread = 0;
uint32_t next_pid = 0;

void switch_task();

void on_timer(isr_state_t* regs)
{
	switch_task();
}

thread_t* get_next_thread()
{
	process_t* next_proc = current_thread->process->next;
	if (next_proc == 0)
		next_proc = k_proc;
	return next_proc->main_thread;
}

void switch_task()
{
	ASSERT(k_proc);
	uint32_t dummy;
	uint32_t* esp_ptr;

	if (current_thread)
	{
		esp_ptr = &current_thread->esp;
		current_thread = get_next_thread();
		//dont switch to same thread
		if (*esp_ptr == current_thread->esp) return;
	}
	else
	{
		//launch first task
		current_thread = k_proc->main_thread;
		//esp_ptr = &dummy;
	}

	current_directory = current_thread->process->pages;
	set_kernel_stack(current_thread->k_stack + KERNEL_STACK_SIZE);

	perform_task_switch(esp_ptr,
		current_thread->esp,
		current_directory->physicalAddr);
}

void idle_task()
{
	for (;;)
	{
		disable_interrupts();
		con_write("Going idle\n");
		enable_interrupts();
	}
}

//called by perform_task_switch() the first time a thread is run
void thread_entry(uint32_t entry)
{
	//setup thread here
	start_user_mode_thread(entry);
	//clean up here
}

thread_t* create_thread(uint32_t entry, uint8_t kernel_mode)
{
	thread_t* t = (thread_t*)kmalloc(sizeof(thread_t));

	t->process = 0;

	t->k_stack = kmalloc(KERNEL_STACK_SIZE);
	//setup the stack so that we will return from perform_task_switch() to the function with the entry point on the stack
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 4) = entry;
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 12) = kernel_mode ? (uint32_t)&start_kernel_mode_thread : (uint32_t)&thread_entry;
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 16) = 0; //ebx
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 20) = 0; //esi
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 24) = 0; //edi
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 28) = t->k_stack; //ebp

	t->esp = t->k_stack + KERNEL_STACK_SIZE - 28;
	return t;
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

	switch_page_directory(kernel_directory);

	p->main_thread = create_thread(entry, 0);
	p->main_thread->process = p;
	
	process_t* tmp = k_proc;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = p;
}

void task_init(page_directory_t* kernel_pages, uint32_t initial_esp)
{
	set_kernel_stack(initial_esp);
	k_proc = (process_t*)kmalloc(sizeof(process_t));
	k_proc->id = next_pid++;
	k_proc->pages = kernel_pages;
	k_proc->heap = kheap;
	k_proc->next = 0;
	k_proc->main_thread = create_thread((uint32_t)&idle_task, 1);
	k_proc->main_thread->process = k_proc;
}

