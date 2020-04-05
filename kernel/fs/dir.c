#include "dir.h"

#include <kernel/memory/kmalloc.h>
#include <kernel/fs/fs.h>

#include <stdio.h>
#include <string.h>

static inline bool _is_dir(const fs_node_t* n)
{
	return n->flags & FS_DIR;
}

static dirent_t* _create_dirent(fs_node_t* n)
{
	dirent_t* dir = (dirent_t*)kmalloc(sizeof(dirent_t));
	memset(dir, 0, sizeof(dirent_t));
	INIT_LIST_HEAD(&dir->child_list);
	dir->node = n;
	return dir;
}

static bool _dir_remove(fs_node_t* parent, fs_node_t* n)
{
	if (!_is_dir(parent))
		return false;
	
	//Remove the dirent_t representing the child from the parent's list
	dirent_t* dir = (dirent_t*)parent->data;
	dirent_t* child;
	list_for_each_entry(child, &dir->child_list, list)
	{
		if (child->node == n)
		{
			list_delete(&child->child_list);
			if (_is_dir(child->node))
				child->node->data = NULL;
			kfree(child);
			return true;
		}
	}
	return false;
}

static int32_t _open_dir(fs_node_t* node, uint32_t flags)
{
	return _is_dir(node) ? 1 : 0;
}

static uint32_t _read_dir(fs_node_t* n, fs_read_dir_cb_fn_t cb, void* data)
{
	uint32_t count = 0;
	dirent_t* dir = (dirent_t*)n->data;
	dirent_t* child;
	list_for_each_entry(child, &dir->child_list, list)
	{
		count++;
		if(!cb(n, child->node, data))
			break;
	}
	return count;
}

static fs_node_t* _find_child(fs_node_t* n, const char* name)
{
	dirent_t* dir = (dirent_t*)n->data;
	dirent_t* child;
	list_for_each_entry(child, &dir->child_list, list)
	{
		if(strcmp(child->node->name, name) == 0)
			return child->node;
	}
	return NULL;
}

static fs_node_t* _dir_add_child(fs_node_t* parent, fs_node_t* child)
{
	if (!_is_dir(parent))
		return NULL;
	
	dirent_t* p_dir = (dirent_t*)parent->data;
	dirent_t* entry = _create_dirent(child);
	if(_is_dir(child))
		child->data = entry;	
	list_add(&entry->list, &p_dir->child_list);
	return entry->node;
}

fs_node_t* fs_create_dir_node(char* name, uint32_t inode)
{
	fs_node_t* n = fs_create_node(name);
	n->inode = inode;
	n->flags |= FS_DIR;
	n->open = &_open_dir;
	n->add_child = &_dir_add_child;
	n->remove_child = &_dir_remove;
	n->read_dir = &_read_dir;
	n->find_child = &_find_child;
	return n;
}

fs_node_t* fs_create_root_node(uint32_t inode)
{
	fs_node_t* root = fs_create_dir_node("", inode);
	root->data = _create_dirent(root);
	return root;
}

static bool _walk_dir(fs_node_t* parent, fs_node_t* n, fs_read_dir_cb_fn_t cb, void* data)
{
	if (parent && !cb(parent, n, data))
		return false;

	if (_is_dir(n))
	{
		dirent_t* dir = (dirent_t*)n->data;
		dirent_t* child;
		list_for_each_entry(child, &dir->child_list, list)
		{
			if (!_walk_dir(n, child->node, cb, data))
				return false;
		}
	}
	return true;
}

void fs_walk_dir(fs_node_t* n, fs_read_dir_cb_fn_t cb, void* data)
{
	if(!_is_dir(n))
		return;
	_walk_dir(NULL, n, cb, data);
}
