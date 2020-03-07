#include "ktimer.h"

#include <kernel/tasks/sched.h>
#include <kernel/time.h>

void ktimer_cb(uint64_t ms)
{
	time_on_tick(ms);
	sched_ontick(ms);
}