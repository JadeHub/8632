#pragma once

#include <stdint.h>

#include <kernel/memory/kheap.h>
#include <kernel/memory/paging.h>

#include <kernel/elf32/elf32.h>

struct thread;

typedef struct process
{
	uint32_t id;
	char name[20];
	page_directory_t* pages;
	heap_t* heap;
	uint32_t entry;
	elf_image_t* elf_img;

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
	uint64_t wake_time;

	//Total time spent executing
	uint64_t cpu_time;

	struct thread* next;
}thread_t;

void proc_init(page_directory_t*, uint32_t);

void proc_new_elf_proc(const char* name, uint8_t* data, uint32_t len);

void proc_switch_to_thread(thread_t* thread, uint32_t* esp_out, uint32_t* ebp_out);

process_t* proc_proc_list();
process_t* proc_kernel_proc();