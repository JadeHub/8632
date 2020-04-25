#include "fatfs.h"

#include <kernel/vfs/vfs.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/types/list.h>

#include <drivers/pci/ide.h>
#include <drivers/fat/fat_access.h>
#include <drivers/fat/fat_table.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct partition_fs
{
	struct fatfs fat;
	ide_device_t* ide_dev;
	//starting location of our partition on the ide device
	uint32_t part_lba;
	fs_node_t* root_node;
}partition_fs_t;

typedef struct dir_info
{
	uint32_t cluster;
	list_head_t children;
}dir_info_t;

static fs_node_t* _root = NULL;
//static fs_node_t* _initrd = NULL;

fs_node_t* _create_dir_node(const char* name, uint32_t cluster);

static size_t _fs_read_file(fs_node_t* node, uint8_t* buff, size_t off, size_t sz)
{
	return 0;
}

static size_t _fs_write_file(fs_node_t* node, uint8_t* buff, size_t off, size_t sz)
{
	return 0;
}

static int32_t _fs_open_file(fs_node_t* node, uint32_t flags)
{
	return 0;
}

static void _fs_close_file(fs_node_t* node)
{
}

static void _populate_dir_children(fs_node_t* node)
{
	dir_info_t* dir = (dir_info_t*)node->data;
	fs_node_t* child;
	if (strcmp(node->name, "fatfs") == 0)
	{
		
		child = _create_dir_node("initrd", 0);
		printf("Adding %s to %s\n", child->name, node->name);
		list_add(&child->list, &dir->children);
	}
}

static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	printf("read dir %s\n", node->name);
	dir_info_t* dir = (dir_info_t*)node->data;

	if (list_empty(&dir->children))
	{
		_populate_dir_children(node);
	}
	fs_node_t* child;
	uint32_t count = 0;
	list_for_each_entry(child, &dir->children, list)
	{
		count++;
		if (!cb(node, child, data))
			break;
	}
	return count;
}

static fs_node_t* _fs_find_child(fs_node_t* node, const char* name)
{
	printf("find child %s in %s\n", name, node->name);

	dir_info_t* dir = (dir_info_t*)node->data;
	if (list_empty(&dir->children))
	{
		_populate_dir_children(node);
	}

	fs_node_t* child;
	list_for_each_entry(child, &dir->children, list)
	{
		if (strcmp(name, child->name) == 0)
		{
			printf("FAT Find Child returns %s 0x%0x\n", child->name, child);
			return child;
		}
	}
	return NULL;
}

fs_node_t* _create_dir_node(const char* name, uint32_t cluster)
{
	fs_node_t* node = fs_create_node(name);
	node->inode = 0;
	node->flags |= FS_DIR;
	node->read_dir = _fs_read_dir;
	node->find_child = _fs_find_child;
	dir_info_t* dir = (dir_info_t*)kmalloc(sizeof(dir_info_t));
	dir->cluster;
	INIT_LIST_HEAD(&dir->children);
	node->data = dir;
	return node;
}

void fatfs_mount_partition(uint8_t ide_controller, uint8_t drive, uint8_t partition)
{
	_root = _create_dir_node("fatfs", 0);
	fs_install_root_fs(_root);
}