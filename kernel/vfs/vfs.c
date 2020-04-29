#include "vfs.h"

#include <kernel/fault.h>
#include <kernel/utils.h>
#include <kernel/types/kname.h>
#include <kernel/memory/kmalloc.h>

#include <stddef.h>
#include <stdio.h>

static fs_node_t* _get_node(fs_node_t* n, char* path, fs_node_t** parent)
{
	
	ASSERT(n);
	char* sep = strchr(path, '/');
	if (sep)
	{
		//Given "blah/blah2/file"
		*sep = '\0';
		sep++;
		//path = "blah"
		//sep = "blah2/file"
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
	fs_node_t* child = fs_find_child(n, path);
	if (child && parent)
		*parent = n;
	return child;
}

int32_t fs_open(fs_node_t* parent, fs_node_t* n)
{
	if (n->open)
		return (*n->open)(parent,n);
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

void fs_flush(fs_node_t* n)
{
	if (n->flush)
		(*n->flush)(n);
}

uint32_t fs_read_dir(fs_node_t* n, fs_read_dir_cb_fn_t cb, void* data)
{
	if (fs_is_dir(n) && n->read_dir)
		return (*n->read_dir)(n, cb, data);
	return 0;
}

fs_node_t* fs_find_child(fs_node_t* n, const char* name)
{
	if (fs_is_dir(n) && n->find_child)
		return (*n->find_child)(n, name);
	return NULL;
}

fs_node_t* fs_create_child(fs_node_t* n, const char* name, uint32_t node_flags)
{
	if (fs_is_dir(n) && n->create_child)
		return (*n->create_child)(n, name, node_flags);
	return NULL;
}

fs_node_t* fs_get_abs_path(const char* path, fs_node_t** parent)
{
	size_t path_len = strlen(path);
	if (path_len > FS_MAX_PATH - 1)
		return NULL;

	if (strcmp(path, "/") == 0)
		return fs_root();

	char* tmp = (char*)kmalloc(strlen(path) + 1);
	strcpy(tmp, path[0] == '/' ? path+1 : path); //skip any initial '/'
	fs_node_t* result = _get_node(fs_root(), tmp, parent);
	kfree(tmp);
	return result;
}