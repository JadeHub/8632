#pragma once

#include <stdint.h>

#include <kernel/memory/kheap.h>
#include <kernel/memory/paging.h>

struct thread;

typedef struct process
{
	uint32_t id;
	char name[20];
	page_directory_t* pages;
	heap_t* heap;
	struct thread* main_thread;
	struct process* next;
}process_t;

typedef enum
{
	TS_RUNNING,
	TS_READY_TO_RUN,
	TS_BLOCKED
} ThreadState;

static inline const char* thread_state_name(ThreadState ts)
{
	switch (ts)
	{
	case TS_BLOCKED:
		return "Blocked";
	case TS_READY_TO_RUN:
		return "Ready to run";
	case TS_RUNNING:
		return "Running";
	}
}

typedef struct thread
{
	uint32_t id;
	process_t* process;
	uint32_t esp;
	uint32_t ebp;
	uint32_t k_stack;
	ThreadState state;
	//struct thread* next;
}thread_t;

void task_init(page_directory_t*, uint32_t);

void task_new_proc(uint8_t* code, uint32_t len);

void task_switch_to_thread(thread_t* thread, uint32_t* esp_out, uint32_t* ebp_out);

process_t* task_proc_list();
process_t* task_kernel_proc();