#include "io.h"

#include <kernel/types/hash_tbl.h>
#include <kernel/fs/fs.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/debug.h>
#include <kernel/types/list.h>
#include <dirent.h>

#include <stdio.h>
#include <string.h>

#define MAX_FD_CNT	64

/*
A file opened by a process
*/
typedef struct proc_file_desc
{
	fs_node_t* node;
	uint32_t flags; //open, r, w, 
	uint64_t offset;
}proc_file_desc_t;

/*
An entry in a directory
*/
typedef struct dir_entry
{
	dirent_t entry;
	list_head_t list_item;
}dir_entry_t;

/*
A Directory opened by a process
*/
typedef struct proc_dir_desc
{
	//dir node
	fs_node_t* node;
	//list of children
	list_head_t child_list;
	//current iteration position
	dir_entry_t* cur;
	//item in proc's list of open DIRs
	list_head_t list_item;
}proc_dir_desc_t;
typedef proc_dir_desc_t DIR;

/*
IO Data for a process
*/
typedef struct proc_io_data
{
	//Process
	process_t* proc;
	//Entry in the hash table to allow lookup by proc id
	hash_tbl_item_t hash_item;
	//Open FDs
	proc_file_desc_t* fds[MAX_FD_CNT];
	//Open Dirs
	list_head_t open_dirs;
}proc_io_data_t;

/*
Map of proc id to proc_io_data_t*
*/
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

uint32_t io_open(const char* path, uint32_t flags)
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
	if (io_is_valid_fd(fd))
	{
		proc->fds[fd] = (proc_file_desc_t*)kmalloc(sizeof(proc_file_desc_t));
		memset(proc->fds[fd], 0, sizeof(proc_file_desc_t));
		proc->fds[fd]->node = node;
		proc->fds[fd]->flags = flags;
		proc->fds[fd]->offset = 0;
		fs_open(node, flags);
	}
	return fd;
}

void io_close(uint32_t fd)
{
	proc_io_data_t* proc = _cur_proc_data();
	proc->fds[fd]->node = NULL;
}

size_t io_read(uint32_t fd, uint8_t* buff, size_t sz)
{
	proc_io_data_t* proc = _cur_proc_data();
	ASSERT(proc->fds[fd]->node);
	size_t read = fs_read(proc->fds[fd]->node, buff, proc->fds[fd]->offset, sz);
	proc->fds[fd]->offset += read;
	return read;
}

size_t io_write(uint32_t fd, const uint8_t* buff, size_t len)
{
	proc_io_data_t* proc = _cur_proc_data();
	ASSERT(proc->fds[fd]->node);
	size_t sz = fs_write(proc->fds[fd]->node, buff, 0, len);
	//proc->fds[fd]->offset += read;
	return sz;
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
	INIT_LIST_HEAD(&res->open_dirs);
	hash_tbl_add(_proc_io, p->id, &res->hash_item);
}

void io_proc_end(process_t* p)
{
	ASSERT(hash_tbl_has(_proc_io, p->id));
	proc_io_data_t* res = hash_tbl_lookup(_proc_io, p->id, proc_io_data_t, hash_item);

	//close any open DIRs
	list_head_t* child = res->open_dirs.next;
	while (child != &res->open_dirs)
	{
		list_head_t* next = child->next;
		proc_dir_desc_t* desc = list_entry(child, proc_dir_desc_t, list_item);
		io_closedir((struct DIR*)desc);
		child = next;
	}

	hash_tbl_delete(_proc_io, p->id);
	kfree(res);
}

bool _dir_read_cb(struct fs_node* parent, struct fs_node* child, void* data)
{
	proc_dir_desc_t* desc = (proc_dir_desc_t*)data;
	ASSERT(parent == desc->node);

	dir_entry_t* item = (dir_entry_t*)kmalloc(sizeof(dir_entry_t));
	strcpy(item->entry.name, child->name);
	item->entry.size = child->len;
	if (fs_is_link(child))
		item->entry.type = DT_LNK;
	else if (fs_is_dir(child))
		item->entry.type = DT_DIR;
	else
		item->entry.type = DT_REG;
	list_add(&item->list_item, &desc->child_list);
	return true;
}

struct DIR* io_opendir(const char* path)
{
	proc_io_data_t* proc = _cur_proc_data();
	//Find the node
	fs_node_t* parent;
	fs_node_t* node = fs_get_abs_path(path, &parent);
	if (!node || (!node->flags & FS_DIR))
		return NULL;

	proc_dir_desc_t* desc = (proc_dir_desc_t*)kmalloc(sizeof(proc_dir_desc_t));
	memset(desc, 0, sizeof(proc_dir_desc_t));
	list_add(&desc->list_item, &proc->open_dirs);
	desc->node = node;
	INIT_LIST_HEAD(&desc->child_list);
	//enum
	fs_read_dir(node, _dir_read_cb, desc);
	if (!list_empty(&desc->child_list))
		desc->cur = list_first_entry(&desc->child_list, dir_entry_t, list_item);
	return (struct DIR*)desc;
}

void io_closedir(struct DIR* dir)
{
	proc_dir_desc_t* desc = (proc_dir_desc_t*)dir;
	list_head_t* child = desc->child_list.next;
	while (child != &desc->child_list)
	{
		list_head_t* next = child->next;
		dir_entry_t* item = list_entry(child, dir_entry_t, list_item);
		kfree(item);
		child = next;
	}
	kfree(desc);
}

struct dirent* io_readdir(struct DIR* dir)
{
	proc_dir_desc_t* desc = (proc_dir_desc_t*)dir;
	if (desc->cur == NULL)
		return NULL;
	dirent_t* result = &desc->cur->entry;
	if (desc->cur->list_item.next == &desc->child_list)
		desc->cur = NULL;
	else
		desc->cur = list_next_entry(desc->cur, list_item);
	return result;
}