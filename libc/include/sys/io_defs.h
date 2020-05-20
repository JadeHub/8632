#pragma once

#include <sys/cdefs.h>

__LIBC_BEGIN_H

#define OPEN_READ	1
#define OPEN_WRITE	2
#define OPEN_CREATE 4
#define OPEN_APPEND 8
#define OPEN_KFILE	16

#define INVALID_FD 0xFFFFFFFF

__LIBC_END_H