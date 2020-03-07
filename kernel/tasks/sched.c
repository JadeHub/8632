#include "sched.h"
#include <kernel/tasks/proc.h>
#include <kernel/x86/interrupts.h>
#include <kernel/fault.h>
#include <kernel/sync/spin_lock.h>
#include <drivers/timer/timer.h>
#include <drivers/console.h>
#include "proc.h"

#include <stdio.h>

#include <stdbool.h>

static uint32_t _sched_slock = 0;
static bool _switching_flag = true;


static thread_t* _ready_list_start = NULL;
static thread_t* _ready_list_end = NULL;
static thread_t* _idle_task = NULL;
static thread_t* _sleep_list = NULL;
static thread_t* _terminated_list = NULL;
static thread_t* _cur_task = NULL;

static thread_t* _next(thread_t* t)
{
	if (t->process->next)
		return t->process->next->main_thread;
	return NULL;
}

static const char* _nameof(thread_t* t)
{
	return t->process->name;
}

static thread_t* _pop_first_ready()
{
	thread_t* task = NULL;

	while (_ready_list_start)
	{
		task = _ready_list_start;
		_ready_list_start = task->next;
		if (_ready_list_start == NULL)
			_ready_list_end = NULL;

		printf("Removed from ready queue %s\n", _nameof(task));
		if (task->state == TS_READY_TO_RUN)
			return task;
	}
	return NULL;
}

static thread_t* _get_next_task()
{
	thread_t* task = _pop_first_ready();

	if (!task)
	{
		if (_cur_task && _cur_task->state == TS_RUNNING)
		{
			task = _cur_task; //keep running current task
		}
		else
		{
			task = _idle_task; //run idle task
		}
	}
	return task;
}

static void _init_switch()
{
	uint32_t dummy;
	uint32_t* esp_ptr = &dummy;
	uint32_t* ebp_ptr = &dummy;
	_cur_task = _idle_task;
	proc_switch_to_thread(_cur_task, esp_ptr, ebp_ptr);
}

static void _switch_task()
{
	//printf("Switching idle 0x%08x cur 0x%08x\n", _idle_task, _cur_task);
	sched_lock();
	if (!_switching_flag)
	{
		sched_unlock();
		return;
	}

	if (!_cur_task)
	{
		_init_switch();
		sched_unlock();
		return;
	}

	uint32_t* esp_ptr;
	uint32_t* ebp_ptr;

	esp_ptr = &_cur_task->esp;
	ebp_ptr = &_cur_task->ebp;
	thread_t* next = _get_next_task();
	if (next == _cur_task)
		return;

	sched_task(_cur_task);
	_cur_task = next;
	
	proc_switch_to_thread(_cur_task, esp_ptr, ebp_ptr);
	sched_unlock();	
}

void sched_init(process_t* kproc)
{
	_idle_task = kproc->main_thread;
}

void sched_ontick(uint64_t ms_since_boot)
{
	thread_t* sleepers = _sleep_list;
	_sleep_list = NULL;
	thread_t* next = sleepers;
	thread_t* t;
	while (next)
	{
		t = next;
		next = t->next;
		if (t->wake_time > 0 && t->wake_time <= ms_since_boot)
		{
			t->wake_time = 0;
			t->state = TS_READY_TO_RUN;
			sched_task(t);
		}
		else
		{
			t->next = _sleep_list;
			_sleep_list = t;
		}		
	}

	_switch_task();
}

void sched_task(thread_t* th)
{
	if (th == _idle_task)
		return;

	if (th->state != TS_READY_TO_RUN)
		return;

	if (_ready_list_end)
	{
		ASSERT(_ready_list_start);
		_ready_list_end->next = th;
		_ready_list_end = th;
	}
	else
	{
		ASSERT(!_ready_list_start);
		_ready_list_start = th;
		_ready_list_end = th;
	}
	printf("Added to ready queue %s\n", _nameof(th));
}

thread_t* sched_cur_thread()
{
	return _cur_task;
}

process_t* sched_cur_proc()
{
	return _cur_task->process;
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

static void _block_cur_task()
{
	sched_lock();
	_cur_task->state = TS_BLOCKED;
	_switch_task();
	sched_unlock();
}

void sched_sleep_until(uint64_t ms)
{
	sched_lock();
	_cur_task->wake_time = ms;
	_cur_task->next = _sleep_list;
	_sleep_list = _cur_task;
	_block_cur_task();
	sched_unlock();
}

void sched_lock()
{
	_sched_slock++;
	//spin_lock(&_sched_slock);
}

void sched_unlock()
{
	_sched_slock--;
	if (_sched_slock == 0)
	{

	}
	//spin_unlock(&_sched_slock);
}

void sched_exit(uint32_t code)
{
	printf("Task end %s %d\n", _nameof(_cur_task), code);
	sched_lock();
	_cur_task->next = _terminated_list;
	_terminated_list = _cur_task;
	_block_cur_task();
	sched_unlock();
}