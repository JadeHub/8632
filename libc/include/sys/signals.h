#pragma once

#include <sys/cdefs.h>
#include <sys/signal_defs.h>

#include <stdint.h>
#include <stdbool.h>

__LIBC_BEGIN_H

void set_sig_handler(uint32_t, sig_handler_t);
bool signal(uint32_t pid, uint32_t sig);

__LIBC_END_H