#include "io.h"

#include <kernel/types/hash_tbl.h>
#include <kernel/vfs/vfs.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/debug.h>
#include <kernel/types/list.h>

#include <sys/io_defs.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define MAX_FD_CNT	64

#define FD_KFLAG_MASK 0x80000000
#define IS_KFILE(_fd) (_fd & FD_KFLAG_MASK)

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

/*
Kernel owned file data
*/
static proc_io_data_t* _kfile_data = NULL;

static proc_io_data_t* _get_proc_data(process_t* proc)
{
	if (hash_tbl_has(_proc_io, proc->id))
		return hash_tbl_lookup(_proc_io, proc->id, proc_io_data_t, hash_item);
	return NULL;
}

static fd_t _find_free_fd(proc_io_data_t* p)
{
	for (int i = 0; i < MAX_FD_CNT; i++)
		if (p->fds[i] == NULL || p->fds[i]->node == NULL)
			return i;
	return INVALID_FD;
}

static void _release_fd(proc_io_data_t* p, fd_t fd)
{
	if(fd != INVALID_FD)
		p->fds[fd]->node = NULL;
}

/*
Find a file decriptor for fnode belonging to proc
*/
static fd_t _find_fd(proc_io_data_t* proc, fs_node_t* fnode)
{
	//dbg_dump_stack();
	for (int i = 0; i < MAX_FD_CNT; i++)
		if (proc->fds[i] && proc->fds[i]->node == fnode)
			return i;
	return INVALID_FD;
}

static proc_file_desc_t* _get_file_desc(uint32_t fd)
{
	proc_io_data_t* data = IS_KFILE(fd) ? _kfile_data : _get_proc_data(sched_cur_proc());
	fd = fd &= ~FD_KFLAG_MASK;
	return data ? data->fds[fd] : NULL;
}

static proc_file_desc_t* _create_file_desc(fs_node_t* node, uint32_t flags)
{
	proc_file_desc_t* desc = (proc_file_desc_t*)kmalloc(sizeof(proc_file_desc_t));
	memset(desc, 0, sizeof(proc_file_desc_t));
	desc->node = node;
	desc->flags = flags;
	desc->offset = 0;
	return desc;
}

bool _dir_empty_cb(struct fs_node* parent, struct fs_node* child, void* data)
{
	return false;
}

static bool _is_dir_empty(fs_node_t* node)
{
	ASSERT(node && fs_is_dir(node));
	return fs_read_dir(node, _dir_empty_cb, NULL) == 0;
}

fd_t _create_file(proc_io_data_t* proc, const char* path, uint32_t flags)
{
	if (io_exists(path))
	{
		printf("File %s exists\n", path);
		return INVALID_FD;
	}

	fd_t fd = INVALID_FD;
	char* dir_name = (char*)kmalloc(strlen(path) + 1);
	strcpy(dir_name, path);
	char* file_name = io_path_split_leaf(dir_name);
	if (!file_name)
		goto _err_exit;	

	fd = _find_free_fd(proc);
	if (!io_is_valid_fd(fd))
		goto _err_exit;
	fs_node_t* parent = fs_get_abs_path(dir_name, NULL);
	if (!parent)
		goto _err_exit;
	printf("Creating child %s in %s\n", file_name, parent->name);
	fs_node_t * node = fs_create_child(parent, file_name, FS_FILE);
	if (!node)
		goto _err_exit;

	proc->fds[fd] = _create_file_desc(node, flags);
	kfree(dir_name);
	return fd;

_err_exit:
	kfree(dir_name);
	_release_fd(proc, fd);
	return INVALID_FD;
}

fd_t _open_file(proc_io_data_t* proc, fs_node_t* parent, fs_node_t* node, uint32_t flags)
{
	fd_t fd = _find_free_fd(proc);
	if (io_is_valid_fd(fd) && fs_open(parent, node))
	{
		proc->fds[fd] = _create_file_desc(node, flags);
		return fd;
	}
	_release_fd(proc, fd);
	return INVALID_FD;
}

bool io_exists(const char* path)
{
	return fs_get_abs_path(path, NULL) != NULL;
}

fd_t io_open(const char* path, uint32_t flags)
{
	proc_io_data_t* proc = (flags & OPEN_KFILE) ? _kfile_data : _get_proc_data(sched_cur_proc());
	ASSERT(proc);

	fd_t fd = INVALID_FD;

	//read
	if (flags & OPEN_READ)
	{
		printf("Opening read\n");
		//Find the node
		fs_node_t* parent;
		fs_node_t* node = fs_get_abs_path(path, &parent);
		if (node)
			fd = _open_file(proc, parent, node, flags);
	}

	//create
	if (fd == INVALID_FD && (flags & OPEN_CREATE))
	{
		fd = _create_file(proc, path, flags);
	}

	//write existing
	if (fd == INVALID_FD && (flags & (OPEN_WRITE | OPEN_APPEND)))
	{
		printf("Opening write append\n");
		//Find the node
		fs_node_t* parent;
		fs_node_t* node = fs_get_abs_path(path, &parent);
		if (node)
			fd = _open_file(proc, parent, node, flags);
	}

	if (fd != INVALID_FD && flags & OPEN_KFILE)
		fd |= FD_KFLAG_MASK;

	return fd;
}

void io_close(fd_t fd)
{
	if (fd == INVALID_FD)
		return;

	proc_io_data_t* proc = IS_KFILE(fd) ? _kfile_data : _get_proc_data(sched_cur_proc());
	ASSERT(proc);
	fd = fd &= ~FD_KFLAG_MASK;
	proc_file_desc_t* file = proc->fds[fd];
	if (!file)
		return;
	fs_close(file->node);
	file->node = NULL;
	_release_fd(proc, fd);
}

size_t io_read(fd_t fd, uint8_t* buff, size_t len)
{
	proc_file_desc_t* file = _get_file_desc(fd);
	if (!file)
		return 0;
	size_t sz = fs_read(file->node, buff, file->offset, len);
	file->offset += sz;
	return sz;
}

size_t io_write(fd_t fd, const uint8_t* buff, size_t len)
{
	proc_file_desc_t* file = _get_file_desc(fd);
	if (!file)
		return 0;	
	size_t sz = fs_write(file->node, buff, file->offset, len);
	file->offset += sz;
	return sz;
}

void io_seek(fd_t fd, uint32_t offset, int origin)
{
	proc_file_desc_t* file = _get_file_desc(fd);
	if (!file)
		return;

	if (offset > file->node->len)
		offset = file->node->len;
	file->offset = offset;
}

void io_flush(fd_t fd)
{
	proc_file_desc_t* file = _get_file_desc(fd);
	if (!file)
		return;
	//todo
}

static proc_io_data_t* _proc_init(process_t* proc)
{
	ASSERT(!hash_tbl_has(_proc_io, proc->id));
	proc_io_data_t* res = (proc_io_data_t*)kmalloc(sizeof(proc_io_data_t));
	memset(res, 0, sizeof(proc_io_data_t));
	for (uint32_t i = 0; i < 64; i++)
		res->fds[i] = NULL;
	res->proc = proc;
	INIT_LIST_HEAD(&res->open_dirs);
	hash_tbl_add(_proc_io, proc->id, &res->hash_item);
	return res;
}

void io_init()
{
	_proc_io = hash_tbl_create(64);

	_kfile_data = (proc_io_data_t*)kmalloc(sizeof(proc_io_data_t));
	memset(_kfile_data, 0, sizeof(proc_io_data_t));
	for (uint32_t i = 0; i < 64; i++)
		_kfile_data->fds[i] = NULL;
}

void io_proc_start(process_t* p, fd_t fds[3])
{
	proc_io_data_t* data = _proc_init(p);

	fs_node_t* node = fs_get_abs_path("/dev/con1", NULL);
	//FD 0 Input 
	if (fds[0] == INVALID_FD)
		data->fds[0] = _create_file_desc(node, OPEN_READ); //input
	else
		io_dup_fd(fds[0], p, 0);

	//FD 1 Output
	if (fds[1] == INVALID_FD)
		data->fds[1] = _create_file_desc(node, OPEN_WRITE); //input
	else
		io_dup_fd(fds[1], p, 1);

	//FD 2 Error
	if (fds[2] == INVALID_FD)
		data->fds[2] = _create_file_desc(node, OPEN_READ); //input
	else
		io_dup_fd(fds[2], p, 2);
}

void io_proc_end(process_t* p)
{
	ASSERT(hash_tbl_has(_proc_io, p->id));
	proc_io_data_t* res = hash_tbl_lookup(_proc_io, p->id, proc_io_data_t, hash_item);

	//destroy any open FDs

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
	proc_io_data_t* proc = _get_proc_data(sched_cur_proc());
	ASSERT(proc);
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
	proc_io_data_t* proc = _get_proc_data(sched_cur_proc());
	ASSERT(proc);
	proc_dir_desc_t* desc = (proc_dir_desc_t*)dir;
	list_head_t* child = desc->child_list.next;
	while (child != &desc->child_list)
	{
		list_head_t* next = child->next;
		dir_entry_t* item = list_entry(child, dir_entry_t, list_item);
		kfree(item);
		child = next;
	}
	list_delete(&desc->list_item);
	kfree(desc);
}

bool io_readdir(struct DIR* dir, struct dirent* ent)
{
	proc_dir_desc_t* desc = (proc_dir_desc_t*)dir;
	if (desc->cur == NULL)
		return false;
	dirent_t* result = &desc->cur->entry;
	if (desc->cur->list_item.next == &desc->child_list)
		desc->cur = NULL;
	else
		desc->cur = list_next_entry(desc->cur, list_item);
	strcpy(ent->name, result->name);
	ent->size = result->size;
	ent->type = result->type;
	return true;
}

int io_mkdir(const char* path)
{
	fd_t fd = INVALID_FD;
	char* dir_name = (char*)kmalloc(strlen(path) + 1);
	strcpy(dir_name, path);
	char* file_name = io_path_split_leaf(dir_name);
	if (!file_name)
		goto _err_exit;
	
	fs_node_t* parent = fs_get_abs_path(dir_name, NULL);
	if (!parent)
		goto _err_exit;
	fs_node_t* node = fs_create_child(parent, file_name, FS_DIR);
	if (!node)
		goto _err_exit;
	kfree(dir_name);
	return 0;
_err_exit:

	kfree(dir_name);
	return -1;
}

int io_remove(const char* path)
{
	/*proc_io_data_t* proc = _get_proc_data(sched_cur_proc());
	ASSERT(proc);*/

	//todo is file open?
	bool result = false;
	fs_node_t* parent;
	fs_node_t* node = fs_get_abs_path(path, &parent);
	
	if (fs_is_dir(node) && !_is_dir_empty(node))
		return -1;

	if (node && node->remove)
		result = (*node->remove)(parent, node);
	return result ? 0 : -1;
}

bool io_dup_fd(fd_t source, process_t* dest_p, fd_t dest)
{
	if (!io_is_valid_fd(source) || !io_is_valid_fd(dest))
		return false;

	proc_file_desc_t* desc = _get_file_desc(source);
	if (!desc)
		return false;

	proc_io_data_t* dest_data = _get_proc_data(dest_p);
	if (dest_data->fds[dest])
	{
		//close?
		kfree(dest_data->fds[dest]);
	}
	dest_data->fds[dest] = _create_file_desc(desc->node, desc->flags);
	return true;
}

/*
given blah/blah/file
	change path to blah/blah
	return file

given blah/blah/
	return NULL
*/
char* io_path_split_leaf(char* path)
{
	char* pos = strrchr(path, FS_SEP_CHAR);
	if (pos == NULL || *(pos+1) == '\0')
		return NULL;
	*pos = '\0';
	return pos+1;
}