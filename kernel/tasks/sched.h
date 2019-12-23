#pragma once

#include <kernel/tasks/proc.h>

/*
Initialise the scheduler
kproc - the kernel process with never ending 'idle' main thread
*/
void sched_init(process_t* kproc);

/*
Returns the currently executing thread
*/
thread_t* sched_cur_thread();

void sched_run();
void sched_pause();

void sched_lock();
void sched_unlock();
