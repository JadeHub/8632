#include "fs.h"

#include <kernel/fs/dir.h>
#include <kernel/fault.h>
#include <kernel/utils.h>

#include <stddef.h>

static fs_node_t* _root = NULL;

static inline bool _is_dir(const fs_node_t* n)
{
	return n->flags & FS_DIR;
}

void fs_init()
{
	_root = fs_create_root_node(0);
}

fs_node_t* fs_root()
{
	return _root;
}

void fs_open(fs_node_t* n, uint32_t flags)
{
	if (n->open)
		(*n->open)(n, flags);
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

size_t fs_write(fs_node_t* n, uint8_t* buff, size_t off, size_t sz)
{
	if (n->write)
		return (*n->write)(n, buff, off, sz);
	return 0;
}

uint32_t fs_read_dir(fs_node_t* n, fs_read_dir_cb_fn_t cb)
{
	if (_is_dir(n) && n->read_dir)
		return (*n->read_dir)(n, cb);
	return 0;
}

fs_node_t* fs_find_child(fs_node_t* n, const char* name)
{
	if (_is_dir(n) && n->find_child)
		return (*n->find_child)(n, name);
	return NULL;
}

fs_node_t* fs_add_child_node(fs_node_t* n, fs_node_t* child)
{
	if (_is_dir(n) && n->add_child)
		return (*n->add_child)(n, child);
	return NULL;
}

bool fs_remove_child_node(fs_node_t* n, fs_node_t* child)
{
	if (_is_dir(n) && n->remove_child)
		return (*n->remove_child)(n, child);
	return false;
}

fs_node_t* fs_install_root_fs(fs_node_t* n)
{
	return fs_add_child_node(_root, n);
}

static fs_node_t* _get_node(fs_node_t* n, const char* path, fs_node_t** parent)
{
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

fs_node_t* fs_get_abs_path(const char* path, fs_node_t** parent)
{
	if (path[0] == '/')
		path++;
	return _get_node(_root, path, parent);
}

