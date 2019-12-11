#pragma once

#include <stdint.h>

#include <kernel/memory/kheap.h>
#include <kernel/memory/paging.h>

struct thread;

typedef struct process
{
	uint32_t id;
	page_directory_t* pages;
	heap_t* heap;
	struct thread* main_thread;
	struct process* next;
}process_t;

typedef struct thread
{
	process_t* process;
	uint32_t ebp;
	uint32_t esp;
	uint32_t eip;
	uint32_t k_stack;
	uint32_t stack_top;
	struct thread* next;
}thread_t;

void task_init(page_directory_t*, uint32_t);

void task_new_proc(uint8_t* code, uint32_t len);