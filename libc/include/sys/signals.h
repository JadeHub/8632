#pragma once

#include <sys/cdefs.h>
#include <sys/signal_defs.h>

#include <stdint.h>

__LIBC_BEGIN_H

void set_sig_handler(uint32_t, sig_handler_t);

__LIBC_END_H