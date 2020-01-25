#pragma once

#include <kernel/tasks/sched.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define INVALID_FD 0xFFFFFFFF;

uint32_t open(const char* path, uint32_t flags);
void close(uint32_t file);
size_t read(uint32_t file, uint8_t* buff, size_t sz);
size_t write(uint32_t file, const uint8_t* buff, size_t sz);

void io_init();

static inline bool io_is_valid_fd(uint32_t fd)
{
	return fd != INVALID_FD;
}

void io_proc_start(process_t* p);
void io_proc_end(process_t* p);