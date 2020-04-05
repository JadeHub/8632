#include "proc.h"
#include "sched.h"

#include <kernel/memory/kmalloc.h>
#include <kernel/memory/paging.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/interrupts.h>
#include <kernel/utils.h>
#include <kernel/fault.h>
#include <kernel/io/io.h>
#include <kernel/debug.h>

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

extern void start_kernel_mode_thread(uint32_t entry);
extern void start_user_mode_thread(uint32_t entry);
extern void perform_task_switch(uint32_t* esp_out, uint32_t esp, uint32_t dir_addr, uint32_t* ebp_out);

extern heap_t* kheap;

#define KERNEL_STACK_SIZE 2048

static process_t* _kproc = 0; //The kernal execution context
static uint32_t next_pid = 0;
static uint32_t next_tid = 0;

void proc_switch_to_thread(thread_t* thread, uint32_t* esp_out, uint32_t* ebp_out)
{
	current_directory = thread->process->pages;
	set_kernel_stack(thread->k_stack + KERNEL_STACK_SIZE);
	thread->state = TS_RUNNING;
	perform_task_switch(esp_out,
		thread->esp,
		current_directory->physicalAddr,
		ebp_out);
}

void idle_task()
{
	for (;;)
	{
		//disable_interrupts();
		//printf("Going idle\n");
		//enable_interrupts();
		asm("HLT");
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

static thread_t* _create_thread(uint32_t entry, bool kernel_mode)
{
	thread_t* t = (thread_t*)kmalloc(sizeof(thread_t));
	memset(t, 0, sizeof(thread_t));

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

	t->state = TS_READY_TO_RUN;
	return t;
}

void _init_proc(process_t* p)
{
	//setup io data for this proc
	io_proc_start(p);
	
	//Add proc to list
	process_t* tmp = _kproc;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = p;

	//schedule the main thread
	sched_task(p->main_thread);
}

void proc_new_elf_proc(const char* name, uint8_t* data, uint32_t len)
{
	process_t* p = (process_t*)kmalloc(sizeof(process_t));
	memset(p, 0, sizeof(process_t));
	p->id = next_pid++;
	sprintf(p->name, "user proc %s", name);
	printf("New proc %s\n", p->name);

	p->pages = clone_directory(kernel_directory);
	switch_page_directory(p->pages);

	bool loaded = elf_load_raw_image(p, name, data, len);
	ASSERT(loaded);

	//heap at 0x00700000
	uint32_t start = 0x00700000;
	uint32_t end = start + 0x100000;
	alloc_pages(p->pages, start, end);
	p->heap = create_heap(p->pages, start, end, end, 0, 0);

	switch_page_directory(kernel_directory);

	p->main_thread = _create_thread(p->entry, false);
	p->main_thread->process = p;

	_init_proc(p);	
}

void proc_init(page_directory_t* kernel_pages, uint32_t initial_esp, elf_image_t* k_elf_image)
{
	set_kernel_stack(initial_esp);
	_kproc = (process_t*)kmalloc(sizeof(process_t));
	memset(_kproc, 0, sizeof(process_t));
	_kproc->id = next_pid++;
	strcpy(_kproc->name, "kidle");
	_kproc->pages = kernel_pages;
	_kproc->heap = kheap;
	_kproc->next = 0;
	_kproc->main_thread = _create_thread((uint32_t)&idle_task, true);
	_kproc->main_thread->process = _kproc;
	_kproc->elf_img = k_elf_image;
}

process_t* proc_kernel_proc()
{
	return _kproc;
}

process_t* proc_proc_list()
{
	return _kproc;
}

