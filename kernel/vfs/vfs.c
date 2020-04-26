#include "vfs.h"

#include <kernel/fault.h>
#include <kernel/utils.h>
#include <kernel/types/kname.h>
#include <kernel/memory/kmalloc.h>

#include <stddef.h>

static inline bool _is_dir(const fs_node_t* n)
{
	return n->flags & FS_DIR;
}

#include <stdio.h>
#include <kernel/tasks/sched.h>

static fs_node_t* _get_node(fs_node_t* n, char* tmp, fs_node_t** parent)
{
	//printf("tmp = 0x%08x proc = 0x%08x\n", tmp, sched_cur_thread()->process);
	char path[FS_MAX_PATH];
	strcpy(path, tmp);
	//printf("Searching '%s'\n", path);
	ASSERT(n);
	char* sep = strchr(path, '/');
	if (sep)
	{
		//Given "blah/blah2/file"
		*sep = '\0';
		sep++;
		//path = "blah"
		//sep = "blah2/file"
		//printf("Finding sub '%s' in '%s'\n", path, n->name);
		fs_node_t* sub = fs_find_child(n, path);
		//handle "blah/" case
		if (*sep == '\0')
		{
			if (parent)
				*parent = n;
			return sub;
		}
		return sub ? _get_node(sub, sep, parent) : NULL;
	}
//	printf("Finding '%s' in '%s'\n", path, n->name);
	fs_node_t* child = fs_find_child(n, path);

	if (child && parent)
		*parent = n;
	return child;
}

int32_t fs_open(fs_node_t* parent, fs_node_t* n, uint32_t flags)
{
	if (n->open)
		return (*n->open)(parent,n, flags);
	return 0;
}

void fs_close(fs_node_t* n)
{
	if (n->close)
		(*n->close)(n);
}

size_t fs_read(fs_node_t* n, uint8_t* buff, size_t off, size_t sz)
{
	if (n->read)
		return (*n->read)(n, buff, off, sz);
	return 0;
}

size_t fs_write(fs_node_t* n, const uint8_t* buff, size_t off, size_t sz)
{
	if (n->write)
		return (*n->write)(n, buff, off, sz);
	return 0;
}

uint32_t fs_read_dir(fs_node_t* n, fs_read_dir_cb_fn_t cb, void* data)
{
	if (_is_dir(n) && n->read_dir)
		return (*n->read_dir)(n, cb, data);
	return 0;
}

fs_node_t* fs_find_child(fs_node_t* n, const char* name)
{
	if (_is_dir(n) && n->find_child)
		return (*n->find_child)(n, name);
	return NULL;
}

fs_node_t* fs_get_abs_path(const char* path, fs_node_t** parent)
{
	if (strlen(path) > FS_MAX_PATH - 1)
		return NULL;
	char tmp[FS_MAX_PATH];
	strcpy(tmp, path);

	if (strcmp(path, "/") == 0)
	//if (path[0] == '/' && path[1] == 0)
		return fs_root();
	
	fs_node_t* res = NULL;
	if (tmp[0] == '/')
		return _get_node(fs_root(), tmp + 1, parent);
	else
		return _get_node(fs_root(), tmp, parent);
}

