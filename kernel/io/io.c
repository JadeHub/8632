#include "io.h"

#include <kernel/types/hash_tbl.h>
#include <kernel/fs/fs.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/debug.h>

#include <stdio.h>
#include <string.h>

#define MAX_FD_CNT	64

typedef struct proc_file_desc
{
	fs_node_t* node;
	uint32_t flags; //open, r, w, 
	uint64_t offset;
}proc_file_desc_t;

typedef struct proc_io_data
{
	process_t* proc;
	hash_tbl_item_t hash_item;
	proc_file_desc_t* fds[MAX_FD_CNT];
}proc_io_data_t;

static hash_tbl_t* _proc_io;

inline static uint32_t _cur_proc_id()
{
	return sched_cur_proc()->id;
}

static proc_io_data_t* _cur_proc_data()
{
	proc_io_data_t* res = hash_tbl_lookup(_proc_io, _cur_proc_id(), proc_io_data_t, hash_item);
	return res;
}

static uint32_t _free_fd(proc_io_data_t* p)
{
	for (int i = 0; i < MAX_FD_CNT; i++)
		if (p->fds[i] == NULL || p->fds[i]->node == NULL)
			return i;
	return INVALID_FD;
}

/*
Find a file decriptor for fnode belonging to proc
*/
static uint32_t _find_fd(proc_io_data_t* proc, fs_node_t* fnode)
{
	//dbg_dump_stack();
	for (int i = 0; i < MAX_FD_CNT; i++)
		if (proc->fds[i] && proc->fds[i]->node == fnode)
			return i;
	return INVALID_FD;
}

uint32_t open(const char* path, uint32_t flags)
{
	proc_io_data_t* proc = _cur_proc_data();
	//Find the node
	fs_node_t* parent;
	fs_node_t* node = fs_get_abs_path(path, &parent);
	if (!node)
		return INVALID_FD;
	//already open?
	uint32_t fd = _find_fd(proc, node);
	if (io_is_valid_fd(fd))
	{
		printf("Already open 0x%x\n", fd);
		return fd;
	}
	fd = _free_fd(proc);
	printf("Open Free 0x%x\n", fd);
	if (io_is_valid_fd(fd))
	{
		proc->fds[fd] = (proc_file_desc_t*)kmalloc(sizeof(proc_file_desc_t));
		memset(proc->fds[fd], 0, sizeof(proc_file_desc_t));
		proc->fds[fd]->node = node;
		proc->fds[fd]->flags = flags;
		proc->fds[fd]->offset = 0;
	}
	return fd;
}

void close(uint32_t fd)
{
	proc_io_data_t* proc = _cur_proc_data();
	proc->fds[fd]->node = NULL;
}

size_t read(uint32_t fd, uint8_t* buff, size_t sz)
{
//	printf("reading 0x%x\n", fd);
	proc_io_data_t* proc = _cur_proc_data();
	ASSERT(proc->fds[fd]->node);
	size_t read = fs_read(proc->fds[fd]->node, buff, proc->fds[fd]->offset, sz);
	proc->fds[fd]->offset += read;
	return read;
}

size_t write(uint32_t file, const uint8_t* buff, size_t len)
{
	return 0;
}

void io_init()
{
	_proc_io = hash_tbl_create(64);
}

void io_proc_start(process_t* p)
{
	ASSERT(!hash_tbl_has(_proc_io, p->id));
	proc_io_data_t* res = (proc_io_data_t*)kmalloc(sizeof(proc_io_data_t));
	memset(res, 0, sizeof(res));
	res->proc = p;
	hash_tbl_add(_proc_io, p->id, &res->hash_item);
}

void io_proc_end(process_t* p)
{
	ASSERT(hash_tbl_has(_proc_io, p->id));
	proc_io_data_t* res = hash_tbl_lookup(_proc_io, p->id, proc_io_data_t, hash_item);
	hash_tbl_delete(_proc_io, p->id);
	kfree(res);
}