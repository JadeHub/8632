#pragma once

#include <sys/signals.h>
#include <kernel/tasks/proc.h>
#include <kernel/x86/interrupts.h>

#include <stdbool.h>

bool sig_queue_signal(process_t*, int);
void sig_handle_pending(isr_state_t*);
void sig_set_handler(process_t* proc, int sig, sig_handler_t handler);
void sig_return(process_t*, isr_state_t*);
