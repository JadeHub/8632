#include "vfs.h"

#include <kernel/fault.h>

#include <string.h>

static fs_node_t* _root = NULL;
static list_head_t _root_child_list;

static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	ASSERT(node == _root);
	fs_node_t* child;
	uint32_t count = 0;
	list_for_each_entry(child, &_root_child_list, list)
	{
		count++;
		if (!cb(node, child, data))
			break;
	}
	return count;
}

static fs_node_t* _fs_find_child(fs_node_t* node, const char* name)
{
	ASSERT(node == _root);
	fs_node_t* child;
	list_for_each_entry(child, &_root_child_list, list)
	{
		if (strcmp(name, child->name) == 0)
			return child;
	}
	return NULL;
}

fs_node_t* fs_root()
{
	ASSERT(_root);
	return _root;
}

fs_node_t* fs_install_root_fs(fs_node_t* n)
{
	list_add(&n->list, &_root_child_list);
	return n;
}

void fs_init()
{
	INIT_LIST_HEAD(&_root_child_list);

	_root = fs_create_node("");
	_root->inode = 0;
	_root->flags |= FS_DIR;
	_root->read_dir = &_fs_read_dir;
	_root->find_child = &_fs_find_child;	
}
