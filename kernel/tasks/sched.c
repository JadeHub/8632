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
//	con_printf("Locked sched\n");
	if (!_switching_flag)
	{
		sched_unlock();
//		con_printf("unlocked sched\n");
		return;
	}

	ASSERT(kernel_proc);
	uint32_t dummy;
	uint32_t* esp_ptr;

	if (current_thread)
	{
		esp_ptr = &current_thread->esp;
		current_thread = _get_next_thread();
		//dont switch to same thread
		if (*esp_ptr == current_thread->esp) return;
	}
	else
	{
		//launch first task
		current_thread = kernel_proc->main_thread;
		esp_ptr = &dummy;
	}
	task_switch_to_thread(current_thread, esp_ptr);
//	con_printf("Unlocked sched\n");
	sched_unlock();	
}

static void _on_timer()
{
	_switch_task();
}

void sched_init(process_t* kproc)
{
	timer_add_callback(&_on_timer);
	kernel_proc = kproc;
}

thread_t* sched_cur_thread()
{
	return current_thread;
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