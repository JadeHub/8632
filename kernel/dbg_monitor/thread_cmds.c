#include "dbg_monitor.h"
#include <kernel/tasks/proc.h>
#include <kernel/tasks/sched.h>
#include <kernel/utils.h>

#include <kernel/debug.h>

#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

static uint32_t _selcted_thread = UINT_MAX;

static void _output_thread(thread_t* t, const char* prefix)
{
	dbg_mon_output_line("%sThread: % 0x%04x", prefix, t->id);
	dbg_mon_output_line("%sThread esp: 0x%08x", prefix, t->esp);
	dbg_mon_output_line("%sThread ebp: 0x%08x", prefix, t->ebp);
	dbg_mon_output_line("%sThread stack: 0x%08x", prefix, t->k_stack);
	dbg_mon_output_line("%sThread state: %s", prefix, thread_state_name(t->state));
}

static thread_t* _find_thread(uint32_t id)
{
	process_t* proc = proc_proc_list();
	while (proc)
	{
		if (proc->main_thread->id == id)
			return proc->main_thread;
		proc = proc->next;
	}
}

static void _stack_unwind_cb(const char* name, uint32_t addr, uint32_t sz, uint32_t ebp, uint32_t ip)
{
	dbg_mon_output_line("0x%08x %-30s at: 0x%08x to 0x%08x ebp: 0x%08x", ip, name, addr, addr + sz, ebp);
}

void threads_cmd(const char* params)
{
	dbg_mon_output_line("Threads command %s", params);

	sched_pause();
	thread_t* cur_thread = sched_cur_thread();
	process_t* proc = proc_proc_list();

	if (!params)
	{
		//List procs and threads
		while (proc)
		{
			dbg_mon_output_line("Process: %04x\t%-20s", proc->id, proc->name);
			dbg_mon_output_line("Page Tables Physical: 0x%08x", proc->pages->tablesPhysical);
			thread_t* t = proc->main_thread;
			_output_thread(t, t == cur_thread ? "\t*" : "\t");
			proc = proc->next;
		}
	}
	else if(isdigit(*params))
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
	else if (streq(params, "s"))
	{
		//stack
		thread_t* t = _find_thread(_selcted_thread);
		if (!t)
		{
			dbg_mon_output_line("No or unknown thread selected");
			return;
		}

		switch_page_directory(t->process->pages);
		dbg_mon_output_line("Unwinding stack...");
		dbg_unwind_stack(dbg_kernel_image(), t->ebp, _stack_unwind_cb);
		switch_page_directory(sched_cur_thread()->process->pages);
	}
	else
	{
		dbg_mon_output_line("Unknown threads command");
	}
	sched_run();
}