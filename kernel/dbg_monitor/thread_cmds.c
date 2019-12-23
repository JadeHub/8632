#include "dbg_monitor.h"
#include <kernel/tasks/task.h>

static void _output_thread(thread_t* t, const char* prefix)
{
	dbg_mon_output_line("%sThread: % %04x", prefix, t->id);
	dbg_mon_output_line("%sThread esp: %08x", prefix, t->esp);
	dbg_mon_output_line("%sThread stack: %08x", prefix, t->k_stack);
	dbg_mon_output_line("%sThread state: %s", prefix, thread_state_name(t->state));
}

void threads_cmd(const char* params)
{
	dbg_mon_output_line("Threads command %s", params);

	//lock
	process_t* proc = task_get_proc_list();
	while (proc)
	{
		dbg_mon_output_line("Process: %04x\t%-20s", proc->id, proc->name);
		dbg_mon_output_line("Page Tables Physical: %08x", proc->pages->tablesPhysical);
		_output_thread(proc->main_thread, "\t");
		proc = proc->next;
	}
}