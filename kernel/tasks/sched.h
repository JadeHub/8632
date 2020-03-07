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

/*
Returns the currently executing process
*/
process_t* sched_cur_proc();

void sched_run();
void sched_pause();

void sched_lock();
void sched_unlock();

void sched_ontick(uint64_t ms_since_boot);

void sched_sleep_until(uint64_t ms);

/*
Add thread to the ready to run list
*/
void sched_task(thread_t* th);

/*
Terminate current thread
*/
void sched_exit(uint32_t code);

//void sched_block();
