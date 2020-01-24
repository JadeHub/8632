#include "io.h"

#include <kernel/tasks/sched.h>
#include <kernel/types/hash_tbl.h>
#include <kernel/fs/fs.h>

typedef struct proc_file_data
{
	fs_node_t* node;
	uint32_t flags; //open, r, w, 
}proc_file_data_t;

typedef struct proc_io_data
{
	process_t* proc;
	//open file data aka fds
	hash_tbl_item_t hash_item;
}proc_io_data_t;

static hash_tbl_t* _proc_io;

inline static uint32_t _cur_proc_id()
{
	return sched_cur_proc()->id;
}

static proc_io_data_t* _cur_proc_data()
{
	proc_io_data_t* res = hash_tbl_lookup(_proc_io, _cur_proc_id(), proc_io_data_t, hash_item);
	if (!res)
	{
		res = (proc_io_data*)kmalloc(sizeof(proc_io_data_t));
		res->proc = sched_cur_proc();
		hash_tbl_add(_proc_io, _cur_proc_id(), &res->hash_item);
	}
	return res;
}

uint32_t open(const char* path, uint32_t flags)
{
	proc_io_data_t* proc = _cur_proc_data();
	return 0;
}

void close(uint32_t file)
{

}

size_t read(uint32_t file, uint8_t* buff, size_t len)
{
	return 0;
}

size_t write(uint32_t file, const uint8_t* buff, size_t len)
{
	return 0;
}

void io_init()
{
	_proc_io = hash_tbl_create(64);
}