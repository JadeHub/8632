#pragma once

#include <sys/cdefs.h>

#include <stdint.h>

__LIBC_BEGIN_H

//Waits for pid to stop, returns exit code
uint32_t wait_pid(uint32_t);

__LIBC_END_H