#include "fatfs.h"

#include <kernel/vfs/vfs.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/types/list.h>

//#include <drivers/fat/fat_access.h>
//#include <drivers/fat/fat_table.h>
//#include <drivers/ide/ide.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/tasks/sched.h>

/*typedef struct partition_fs
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
}dir_info_t;*/

fs_node_t* initrd = NULL;
fs_node_t* _root_node = NULL;


fs_node_t* _create_dir_node(const char* name, uint32_t cluster);


static void _populate_dir_data(fs_node_t* dir_node)
{
	printf("FAT Populate %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	ASSERT(fs_is_dir(dir_node));
	if (dir_node == _root_node && initrd == NULL)
	{
		printf("FAT Populate creating\n");
		initrd = _create_dir_node("initrd", 0);
		printf("FAT Populate created\n");
	}
	else
	{
	// fs	ASSERT(0);
	}

	printf("FAT Populate END %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
}

fs_node_t* _fs_find_child(fs_node_t* node, const char* name)
{
	printf("FAT Find Child %s in %s _cut_task==0x%x proc==0x%x id=%d\n", name, node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	if (node == _root_node && strcmp(name, "initrd") == 0)
	{
		if (initrd == NULL)
		{
			_populate_dir_data(node);
		}
		printf("FAT Find Child returning %s in %s _cut_task==0x%x proc==0x%x id=%d\n", name, node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
		return initrd;
	}

	//ASSERT(0);
	return NULL;
}

uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	if (node == _root_node)
	{
		if (initrd == NULL)
		{
			_populate_dir_data(node);
		}
		cb(node, initrd, data);
		return 1;
	}
	return 0;
}


fs_node_t* _create_dir_node(const char* name, uint32_t cluster)
{
	fs_node_t* node = fs_create_node(name);
	node->inode = 0;
	node->flags |= FS_DIR;
	node->read_dir = _fs_read_dir;
	node->find_child = _fs_find_child;
	return node;
}

void fatfs_mount_partition(uint8_t ide_controller, uint8_t drive, uint8_t part)
{
	_root_node = _create_dir_node("fat32", 0);
	initrd = _create_dir_node("initrd", 0);
	//_populate_dir_data(_root_node);
	fs_install_root_fs(_root_node);

	printf("Mounting partition\n");
}

#if 0

typedef struct cluster_lookup
{
	uint32 index;
	uint32 cluster;
}cluster_lookup_t;

typedef struct file_info
{
	uint32_t start_cluster;
	uint32_t length;

	cluster_lookup_t last_fat_lookup;

	// Read/Write sector buffer
	uint8_t file_data_sector[FAT_SECTOR_SIZE];
	uint32_t file_data_address;
	bool file_data_dirty;

}file_info_t;

static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data);
static fs_node_t* _fs_find_child(fs_node_t* node, const char* name);

static partition_fs_t* _mounted_partition = NULL;

static struct fatfs* _get_fatfs(fs_node_t* node)
{
	return &_mounted_partition->fat;
}

static fs_node_t* _create_dir_node(const char* name, uint32_t cluster)
{
	fs_node_t* node = fs_create_node(name);
	node->inode = 0;
	node->flags |= FS_DIR;
	dir_info_t* info = (dir_info_t*)kmalloc(sizeof(dir_info_t));
	info->cluster = cluster;
	INIT_LIST_HEAD(&info->children);
	node->data = info;
	node->read_dir = _fs_read_dir;
	node->find_child = _fs_find_child;
	return node;
}

/*
VFS Interface
*/

/*static uint32_t _read_sectors(struct fatfs* fatfs, file_info_t* file, uint8_t* buffer, uint32_t offset, uint32_t size)
{
	uint32_t sector = 0;
	uint32_t cluster_idx = 0;
	uint32_t cluster = 0;
	uint32_t i;
	uint32_t lba;

	// Find cluster index within file & sector with cluster
	cluster_idx = offset / fatfs->sectors_per_cluster;
	sector = offset - (cluster_idx * fatfs->sectors_per_cluster);

	// Limit number of sectors read to the number remaining in this cluster
	if ((sector + size) > fatfs->sectors_per_cluster)
		size = fatfs->sectors_per_cluster - sector;

	// Quick lookup for next link in the chain
	if (cluster_idx == file->last_fat_lookup.index)
		cluster = file->last_fat_lookup.cluster;
	// Else walk the chain
	else
	{
		// Starting from last recorded cluster?
		if (cluster_idx && cluster_idx == file->last_fat_lookup.index + 1)
		{
			i = file->last_fat_lookup.index;
			cluster = file->last_fat_lookup.cluster;
		}
		// Start searching from the beginning..
		else
		{
			// Set start of cluster chain to initial value
			i = 0;
			cluster = file->start_cluster;
		}

		// Follow chain to find cluster to read
		for (; i < cluster_idx; i++)
		{
			uint32_t next_cluster;

			// Does the entry exist in the cache?
		//	if (!fatfs_cache_get_next_cluster(fatfs, file, i, &nextCluster))
			{
				// Scan file linked list to find next entry
				next_cluster = fatfs_find_next_cluster(fatfs, cluster);

				// Push entry into cache
			//	fatfs_cache_set_next_cluster(fatfs, file, i, nextCluster);
			}

			cluster = next_cluster;
		}

		// Record current cluster lookup details (if valid)
		if (cluster != FAT32_LAST_CLUSTER)
		{
			file->last_fat_lookup.cluster = cluster;
			file->last_fat_lookup.index = cluster_idx;
		}
	}

	// If end of cluster chain then return false
	if (cluster == FAT32_LAST_CLUSTER)
		return 0;

	// Calculate sector address
	lba = fatfs_lba_of_cluster(fatfs, cluster) + sector;

	// Read sector of file
	return fatfs_sector_read(fatfs, lba, buffer, size);
}*/

static size_t _fs_read_file(fs_node_t* node, uint8_t* buffer, size_t off, size_t count)
{
	/*ASSERT(node && buffer);
	struct fatfs* fatfs = _get_fatfs(node);
	//Open?
	if (node->data == NULL)
		return 0;
	file_info_t* file = (file_info_t*)node->data;

	if (off >= file->length)
		return 0;

	if (off + count > file->length)
		count = file->length - off;

	uint32_t sector;
	uint32_t offset;

	// Calculate start sector
	sector = off / FAT_SECTOR_SIZE;

	// Offset to start copying data from first sector
	offset = off % FAT_SECTOR_SIZE;

	uint32_t bytesRead = 0;
	uint32_t copy_count;
	while (bytesRead < count)
	{
		// Read whole sector, read from media directly into target buffer
		if ((offset == 0) && ((count - bytesRead) >= FAT_SECTOR_SIZE))
		{
			// Read as many sectors as possible into target buffer
			uint32 sectorsRead = _read_sectors(fatfs, file, (uint8_t*)(buffer + bytesRead), sector, (count - bytesRead) / FAT_SECTOR_SIZE);
			if (sectorsRead)
			{
				// We have upto one sector to copy
				copy_count = FAT_SECTOR_SIZE * sectorsRead;

				// Move onto next sector and reset copy offset
				sector += sectorsRead;
				offset = 0;
			}
			else
				break;
		}
		else
		{
			// Do we need to re-read the sector?
			if (file->file_data_address != sector)
			{
				// Flush un-written data to file
			//	if (file->file_data_dirty)
			//		fl_fflush(file);

				// Get LBA of sector offset within file
				if (!_read_sectors(fatfs, file, file->file_data_sector, sector, 1))
					// Read failed - out of range (probably)
					break;

				file->file_data_address = sector;
				file->file_data_dirty = 0;
			}

			// We have upto one sector to copy
			copy_count = FAT_SECTOR_SIZE - offset;

			// Only require some of this sector?
			if (copy_count > (count - bytesRead))
				copy_count = (count - bytesRead);

			// Copy to application buffer
			memcpy((uint8*)((uint8*)buffer + bytesRead), (uint8*)(file->file_data_sector + offset), copy_count);

			// Move onto next sector and reset copy offset
			sector++;
			offset = 0;
		}

		// Increase total read count
		bytesRead += copy_count;
	}

	return bytesRead;*/
	return 0;
}

static size_t _fs_write_file(fs_node_t* node, uint8_t* buff, size_t off, size_t sz)
{
	return 0;
}

static int32_t _fs_open_file(fs_node_t* parent, fs_node_t* node, uint32_t flags)
{
	/*ASSERT(fs_is_dir(parent) && !fs_is_dir(node));
	dir_info_t* dir = (dir_info_t*)parent->data;
	struct fatfs* fatfs = _get_fatfs(node);

	if (node->data)
		return -1; //open
	
	struct fat_dir_entry sfEntry;
	if (fatfs_get_file_entry(fatfs, dir->cluster, node->name, &sfEntry))
	{
		file_info_t* file = (file_info_t*)kmalloc(sizeof(file_info_t));
		file->start_cluster = ((FAT_HTONS((uint32)sfEntry.FstClusHI)) << 16) + FAT_HTONS(sfEntry.FstClusLO);
		file->length = FAT_HTONL(sfEntry.FileSize);
		node->data = file;

		file->last_fat_lookup.index = 0xFFFFFFFF;
		file->last_fat_lookup.cluster = 0xFFFFFFFF;

		file->file_data_address = 0xFFFFFFFF;
		file->file_data_dirty = 0;
		return 0;
	}*/
	return -1;
}

static void _fs_close_file(fs_node_t* node)
{
/*	file_info_t* file = (file_info_t*)node->data;
	kfree(node->data);
	node->data = NULL;*/
}

static bool _fs_remove_file(fs_node_t* node)
{
	return false;
}

static void _populate_dir_data(fs_node_t* dir_node)
{
	printf("FAT Populate %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	ASSERT(fs_is_dir(dir_node));
	dir_info_t* info = (dir_info_t*)dir_node->data;
	ASSERT(info);
	fs_node_t* entry = NULL;
	if (strcmp(dir_node->name, "fat32") == 0)
	{
		entry = _create_dir_node("initrd", 0);
		list_add(&entry->list, &info->children);
	}
	if (strcmp(dir_node->name, "initrd") == 0)
	{
		entry = _create_dir_node("test", 0);
		list_add(&entry->list, &info->children);
	}

	printf("FAT Populate END %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	/*printf("FAT Populate %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	ASSERT(fs_is_dir(dir_node));
	dir_info_t* info = (dir_info_t*)dir_node->data;

	struct fs_dir_list_status dirls;
	struct fatfs* fatfs = _get_fatfs(dir_node);

	printf("FAT Populate 1 %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
	fatfs_list_directory_start(fatfs, &dirls, info->cluster);

	printf("FAT Populate 2 %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	struct fs_dir_ent dirent;
	while (fatfs_list_directory_next(fatfs, &dirls, &dirent))
	{
		fs_node_t* entry = NULL;
		if (dirent.is_dir)
		{
			printf("FAT Populate Adding %s _cut_task==0x%x proc==0x%x id=%d\n", dirent.filename, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

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

	printf("FAT END Populate %s _cut_task==0x%x proc==0x%x id=%d\n", dir_node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
	*/
}

static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	ASSERT(fs_is_dir(node));
	printf("FAT ReadDir %s _cut_task==0x%x proc==0x%x id=%d\n", node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
	_populate_dir_data(node);
	/*

	dir_info_t* dir = (dir_info_t*)node->data;
	if (list_empty(&dir->children))
		_populate_dir_data(node);

	fs_node_t* child;
	uint32_t count = 0;
	list_for_each_entry(child, &dir->children, list)
	{
		count++;
		if (!cb(node, child, data))
			break;
	}
	printf("FAT ReadDir END %s _cut_task==0x%x proc==0x%x id=%d\n", node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
	*/

	//if(strecm)

	//cb(node, _create_dir_node("initrd", 0), data);

	//return count;
	return 1;
}

#include <kernel/utils.h>

static fs_node_t* _fs_find_child(fs_node_t* node, const char* name)
{
	ASSERT(node);
	printf("FAT Find Child %s in %s _cut_task==0x%x proc==0x%x id=%d\n", name, node->name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	bochs_dbg();
	//dir_info_t* dir = (dir_info_t*)node->data;
	bochs_dbg();
	printf("FAT Find Child 0 proc==0x%x data=0x%x\n", sched_cur_thread(), node);
	bochs_dbg();
	/*if (list_empty(&dir->children))
	{
		printf("FAT Find Child 1 %s _cut_task==0x%x proc==0x%x id=%d\n", name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);
		_populate_dir_data(node);
	}
	printf("FAT Find Child 2 %s _cut_task==0x%x proc==0x%x id=%d\n", name, sched_cur_thread(), sched_cur_thread()->process, sched_cur_thread()->process->id);

	fs_node_t* child;
	list_for_each_entry(child, &dir->children, list)
	{
		if (strcmp(name, child->name) == 0)
		{
			printf("FAT Find Child returns %s 0x%0x\n", child->name, child);
			return child;
		}
	}*/
	return NULL;
}



/*
IDE Interface
*/

static int _disk_read(unsigned long sector, unsigned char* buffer, unsigned long sector_count)
{
//	printf("Disk read\n");
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

	/*if (fatfs_init(&_mounted_partition->fat) != FAT_INIT_OK)
	{
		ASSERT(false);
	}*/

	//create root node
	_mounted_partition->root_node = _create_dir_node("fat32",
			_mounted_partition->fat.rootdir_first_cluster); //cluster?
	fs_install_root_fs(_mounted_partition->root_node);

	printf("Mounting partition of type %s at offset 0x%x\n",
			_mounted_partition->fat.fat_type == FAT_TYPE_32 ? "fat32" : "fat16",
			partition->lba);
}
#endif