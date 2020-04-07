#pragma once

#include <kernel/tasks/sched.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define INVALID_FD 0xFFFFFFFF;

//open flags
#define IO_OPEN_R	0x01
#define IO_OPEN_W	0x02
#define IO_OPEN_RW	0x03

uint32_t io_open(const char* path, uint32_t flags);
void io_close(uint32_t file);
size_t io_read(uint32_t file, uint8_t* buff, size_t sz);
size_t io_write(uint32_t file, const uint8_t* buff, size_t sz);

void io_init();

static inline bool io_is_valid_fd(uint32_t fd)
{
	return fd != INVALID_FD;
}

void io_proc_start(process_t* p);
void io_proc_end(process_t* p);


struct DIR;
struct dirent;

struct DIR* io_opendir(const char*);
void io_closedir(struct DIR*);
struct dirent* io_readdir(struct DIR*);