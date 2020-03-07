#include "sched.h"
#include <kernel/tasks/proc.h>
#include <kernel/x86/interrupts.h>
#include <kernel/fault.h>
#include <kernel/sync/spin_lock.h>
#include <drivers/timer/timer.h>
#include <drivers/console.h>
#include "proc.h"

#include <stdbool.h>

static process_t* kernel_proc = 0;
static thread_t* current_thread = 0;
static uint32_t sched_slock = 0;
static bool _switching_flag = true;

static thread_t* _get_next_thread()
{
	process_t* next_proc = current_thread->process->next;
	if (next_proc == 0)
		next_proc = kernel_proc;
	return next_proc->main_thread;
}

static void _switch_task()
{
	sched_lock();
	if (!_switching_flag)
	{
		sched_unlock();
		return;
	}

	ASSERT(kernel_proc);
	uint32_t dummy;
	uint32_t* esp_ptr;
	uint32_t* ebp_ptr;

	if (current_thread)
	{
		esp_ptr = &current_thread->esp;
		ebp_ptr = &current_thread->ebp;
		current_thread = _get_next_thread();
		//dont switch to same thread
		if (*esp_ptr == current_thread->esp) return;
	}
	else
	{
		//launch first task
		current_thread = kernel_proc->main_thread;
		esp_ptr = &dummy;
		ebp_ptr = &dummy;
	}
	proc_switch_to_thread(current_thread, esp_ptr, ebp_ptr);
	sched_unlock();	
}

static void _on_timer()
{
	//_switch_task();
}

void sched_init(process_t* kproc)
{
	//timer_add_callback(&_on_timer);
	kernel_proc = kproc;
}

void sched_ontick(uint64_t ms_since_boot)
{
	_switch_task();
}

thread_t* sched_cur_thread()
{
	return current_thread;
}

process_t* sched_cur_proc()
{
	return current_thread->process;
}

void sched_run()
{
	sched_lock();
	_switching_flag = true;
	sched_unlock();
}

void sched_pause()
{
	sched_lock();
	_switching_flag = false;
	sched_unlock();
}

void sched_lock()
{
	//spin_lock(&sched_slock);
}

void sched_unlock()
{
	//spin_unlock(&sched_slock);
}