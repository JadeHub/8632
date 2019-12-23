#include "dbg_monitor.h"
#include <kernel/tasks/proc.h>
#include <kernel/tasks/sched.h>
#include <kernel/utils.h>

#include <limits.h>

static uint32_t _selcted_thread = UINT_MAX;

static void _output_thread(thread_t* t, const char* prefix)
{
	dbg_mon_output_line("%sThread: % %04x", prefix, t->id);
	dbg_mon_output_line("%sThread esp: %08x", prefix, t->esp);
	dbg_mon_output_line("%sThread stack: %08x", prefix, t->k_stack);
	dbg_mon_output_line("%sThread state: %s", prefix, thread_state_name(t->state));
}

static thread_t* _find_thread(uint32_t id)
{
	process_t* proc = task_proc_list();
	while (proc)
	{
		if (proc->main_thread->id == id)
			return proc->main_thread;
		proc = proc->next;
	}
}

void threads_cmd(const char* params)
{
	dbg_mon_output_line("Threads command %s", params);

	sched_pause();
	thread_t* cur_thread = sched_cur_thread();
	process_t* proc = task_proc_list();

	if (!params)
	{
		//List procs and threads
		while (proc)
		{
			dbg_mon_output_line("Process: %04x\t%-20s", proc->id, proc->name);
			dbg_mon_output_line("Page Tables Physical: %08x", proc->pages->tablesPhysical);
			thread_t* t = proc->main_thread;
			_output_thread(t, t == cur_thread ? "\t*" : "\t");
			proc = proc->next;
		}
	}
	else if(is_digit(*params))
	{
		//Select thread
		int id = atoi(params);
		thread_t* t = _find_thread(id);
		if (t)
			_selcted_thread = id;
		else
			dbg_mon_output_line("Unknown thread %04x", id);
		if(_selcted_thread != UINT32_MAX)
			dbg_mon_output_line("Selected thread %04x", _selcted_thread);
	}
	sched_run();
}