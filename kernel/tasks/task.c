#include "task.h"

#include <kernel/memory/kheap.h>
#include <kernel/memory/paging.h>
#include <kernel/x86/interrupts.h>
#include <kernel/x86/gdt.h>
#include <kernel/utils.h>

extern void move_stack(void *new_stack_start, uint32_t size, uint32_t initial_esp);

#define KERNEL_STACK_SIZE 2048

process_t* k_proc; //The kernal execution context
thread_t* current_thread = 0;

void switch_task()
{

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
	k_proc->main_thread->ebp = 0;
	k_proc->main_thread->esp = 0;
	k_proc->main_thread->eip = 0;
	k_proc->main_thread->k_stack = initial_esp - KERNEL_STACK_SIZE;
	k_proc->main_thread->next = 0;
	current_thread = k_proc->main_thread;
    enable_interrupts();
}

