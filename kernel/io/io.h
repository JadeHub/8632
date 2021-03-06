#pragma once

#include <sys/io_defs.h>
#include <kernel/tasks/sched.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//open flags
/*
#define IO_OPEN_R	0x01
#define IO_OPEN_W	0x02
#define IO_OPEN_RW	0x03
#define IO_KFILE	0x04
*/

typedef uint32_t fd_t;

void io_init();
fd_t io_open(const char* path, uint32_t flags);
void io_close(fd_t file);
size_t io_read(fd_t file, uint8_t* buff, size_t sz);
size_t io_write(fd_t file, const uint8_t* buff, size_t sz);
void io_seek(fd_t file, uint32_t offset, int origin);
void io_flush(fd_t);
int io_remove(const char* path);
int io_mkdir(const char* path);
bool io_exists(const char* path);

/*
Duplicate source fd from proc as dest fd for the current process
*/
bool io_dup_fd(fd_t source, process_t* dest_p, fd_t dest);


static inline bool io_is_valid_fd(fd_t fd)
{
	return fd != INVALID_FD;
}

void io_proc_start(process_t* p, fd_t fds[3]);
void io_proc_end(process_t* p);


struct DIR;
struct dirent;

struct DIR* io_opendir(const char*);
void io_closedir(struct DIR*);
bool io_readdir(struct DIR*, struct dirent*);

char* io_path_split_leaf(char* path);