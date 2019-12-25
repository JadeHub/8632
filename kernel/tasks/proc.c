#include "proc.h"
#include "sched.h"

#include <kernel/memory/kmalloc.h>
#include <kernel/memory/paging.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/interrupts.h>
#include <kernel/utils.h>
#include <kernel/fault.h>
#include <drivers/console.h>

#include <string.h>

extern void start_kernel_mode_thread(uint32_t entry);
extern void start_user_mode_thread(uint32_t entry);
extern void perform_task_switch(uint32_t* esp_out, uint32_t esp, uint32_t dir_addr, uint32_t* ebp_out);

extern heap_t* kheap;

#define KERNEL_STACK_SIZE 2048

static process_t* kernel_proc = 0; //The kernal execution context
static uint32_t next_pid = 0;
static uint32_t next_tid = 0;

void task_switch_to_thread(thread_t* thread, uint32_t* esp_out, uint32_t* ebp_out)
{
	current_directory = thread->process->pages;
	set_kernel_stack(thread->k_stack + KERNEL_STACK_SIZE);

	perform_task_switch(esp_out,
		thread->esp,
		current_directory->physicalAddr,
		ebp_out);
}

void test5()
{
	asm("HLT");
}

void test1a()
{
	int t = 5;
	test5();
}

void testa()
{
	for (int i = 0;i < 100000; i++)
	{
		;
	}
	test1a();
}

void idle_task()
{
	for (;;)
	{
		//disable_interrupts();
		//con_write("Going idle\n");
		//enable_interrupts();
		testa();

	}
}

void kernel_thread_entry(uint32_t entry)
{
	sched_unlock();
	start_kernel_mode_thread(entry);
}

//called by perform_task_switch() the first time a thread is run
void user_thread_entry(uint32_t entry)
{
	//setup thread here
	sched_unlock();
	start_user_mode_thread(entry);
}

static thread_t* _create_thread(uint32_t entry, uint8_t kernel_mode)
{
	thread_t* t = (thread_t*)kmalloc(sizeof(thread_t));

	t->process = 0;
	t->id = next_tid++;
	t->k_stack = kmalloc(KERNEL_STACK_SIZE);
	//setup the stack so that we will return from perform_task_switch() to the function with the entry point on the stack
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 4) = entry;
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 12) = kernel_mode ? (uint32_t)&kernel_thread_entry : (uint32_t)&user_thread_entry;
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 16) = 0; //ebx
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 20) = 0; //esi
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 24) = 0; //edi
	*(uint32_t*)(t->k_stack + KERNEL_STACK_SIZE - 28) = 0;// t->k_stack; //ebp

	t->esp = t->k_stack + KERNEL_STACK_SIZE - 28;
	return t;
}

void task_new_proc(uint8_t* code, uint32_t len)
{
	//disable_interrupts();

	process_t* p = (process_t*)kmalloc(sizeof(process_t));
	p->id = next_pid++;
	sprintf(p->name, "user proc %x", p->id);
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

	p->main_thread = _create_thread(entry, 0);
	p->main_thread->process = p;
	
	process_t* tmp = kernel_proc;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = p;
}

void task_init(page_directory_t* kernel_pages, uint32_t initial_esp)
{
	set_kernel_stack(initial_esp);
	kernel_proc = (process_t*)kmalloc(sizeof(process_t));
	kernel_proc->id = next_pid++;
	strcpy(kernel_proc->name, "kidle");
	kernel_proc->pages = kernel_pages;
	kernel_proc->heap = kheap;
	kernel_proc->next = 0;
	kernel_proc->main_thread = _create_thread((uint32_t)&idle_task, 1);
	kernel_proc->main_thread->process = kernel_proc;
}

process_t* task_kernel_proc()
{
	return kernel_proc;
}

process_t* task_proc_list()
{
	return kernel_proc;
}
