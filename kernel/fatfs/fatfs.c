#include "fatfs.h"

#include <kernel/vfs/vfs.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/types/list.h>

#include <drivers/ide/ide.h>
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

static partition_fs_t* _mounted_partition = NULL;

static struct fatfs* _get_fatfs(fs_node_t* node)
{
	return &_mounted_partition->fat;
}

/*
VFS interface
*/

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

static void _populate_dir_children(fs_node_t* dir_node)
{
	/*dir_info_t* dir = (dir_info_t*)node->data;
	fs_node_t* child;
	if (strcmp(node->name, "fatfs") == 0)
	{
		
		child = _create_dir_node("initrd", 0);
		printf("Adding %s to %s\n", child->name, node->name);
		list_add(&child->list, &dir->children);
	}*/
	ASSERT(fs_is_dir(dir_node));
	dir_info_t* info = (dir_info_t*)dir_node->data;

	struct fs_dir_list_status dirls;
	struct fatfs* fatfs = _get_fatfs(dir_node);

	//printf("FAT Populate 1 %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
	fatfs_list_directory_start(fatfs, &dirls, info->cluster);

	//printf("FAT Populate 2 %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	struct fs_dir_ent dirent;
	while (fatfs_list_directory_next(fatfs, &dirls, &dirent))
	{
		fs_node_t* entry = NULL;
		if (dirent.is_dir)
		{
			printf("FAT Populate Adding %s \n", dirent.filename);

			entry = _create_dir_node(dirent.filename, dirent.cluster);
			list_add(&entry->list, &info->children);
		}
		else
		{
			//entry = fs_create_node(dirent.filename);
			//entry->len = dirent.size;
		//	entry->read = _fs_read_file;
		//	entry->open = _fs_open_file;
			//entry->write = _fs_write_file;
		//	entry->close = _fs_close_file;
		}

	}

}

static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	dir_info_t* dir = (dir_info_t*)node->data;

	if (list_empty(&dir->children))
		_populate_dir_children(node);
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
	dir_info_t* dir = (dir_info_t*)node->data;
	if (list_empty(&dir->children))
		_populate_dir_children(node);

	fs_node_t* child;
	list_for_each_entry(child, &dir->children, list)
	{
		if (strcmp(name, child->name) == 0)
		{
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
	dir->cluster = cluster;
	INIT_LIST_HEAD(&dir->children);
	node->data = dir;
	return node;
}

/*
IDE Interface
*/

static int _disk_read(unsigned long sector, unsigned char* buffer, unsigned long sector_count)
{
	return ide_read_sectors(_mounted_partition->ide_dev, sector_count, _mounted_partition->part_lba + sector, buffer);
}

static int _disk_write(unsigned long sector, unsigned char* buffer, unsigned long sector_count)
{
	return ide_write_sectors(_mounted_partition->ide_dev, sector_count, _mounted_partition->part_lba + sector, buffer);
}

void fatfs_mount_partition(uint8_t ide_controller, uint8_t drive, uint8_t part)
{
	ide_device_t* ide = ide_get_device(ide_controller, drive);
	if (!ide)
	{
		ASSERT(ide);
		return;
	}

	ASSERT(ide->type == 0);

	ide_partition_t* partition = &ide->partitions[part];
	if (!partition->present)
	{
		ASSERT(ide);
		return;
	}

	_mounted_partition = (partition_fs_t*)kmalloc(sizeof(partition_fs_t));
	memset(_mounted_partition, 0, sizeof(partition_fs_t));
	_mounted_partition->ide_dev = ide;
	_mounted_partition->part_lba = partition->lba;
	_mounted_partition->fat.disk_io.read_media = _disk_read;
	_mounted_partition->fat.disk_io.write_media = _disk_write;

	printf("Mounting partition %d lba= 0x%x\n", part, _mounted_partition->part_lba);

	int r = fatfs_init(&_mounted_partition->fat);
	if (r != FAT_INIT_OK)
	{
		printf("Error %d\n", r);
		ASSERT(false);
	}

	//create root node
	_mounted_partition->root_node = _create_dir_node("fatfs",
		_mounted_partition->fat.rootdir_first_cluster);
	fs_install_root_fs(_mounted_partition->root_node);

	printf("Mounting partition of type %s at offset 0x%x\n",
		_mounted_partition->fat.fat_type == FAT_TYPE_32 ? "fat32" : "fat16",
		partition->lba);
}