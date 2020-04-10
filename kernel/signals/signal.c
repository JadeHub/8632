#include "signal.h"

#include <kernel/tasks/sched.h>
#include <kernel/tasks/proc.h>
#include <kernel/fault.h>

void sig_set_handler(process_t* proc, int sig, sig_handler_t handler)
{
	proc->sig_handlers[sig - 1] = handler;
}

bool sig_queue_signal(process_t* proc, int sig)
{
	if (!proc->sig_handlers[sig - 1])
		return false;

	proc->pending_signals[sig-1] = true;
	return true;
}

void sig_handle_pending(isr_state_t* istate)
{
	process_t* proc = sched_cur_proc();
	if (!proc || proc == proc_kernel_proc() || proc->in_sig)
		return;
	for (int i = 0; i < SIG_COUNT; i++)
	{
		if (proc->pending_signals[i] && proc->sig_handlers[i])
		{
			proc->pending_signals[i] = false;
			proc->sig_return_state = *istate;
			proc->in_sig = true;
			istate->eip = (uint32_t)proc->sig_handlers[i];
			break;
		}
	}
}

void sig_return(process_t* proc, isr_state_t* istate)
{
	ASSERT(proc->in_sig);
	proc->in_sig = false;
	*istate = proc->sig_return_state;
}