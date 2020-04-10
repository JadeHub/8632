#pragma once

#include <sys/cdefs.h>

__LIBC_BEGIN_H

#define SIG_COUNT 2

#define SIGINT 1
#define SIGKILL 2

typedef void (*sig_handler_t)(void);

__LIBC_END_H