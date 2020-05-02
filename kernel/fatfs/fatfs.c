#include "fatfs.h"

#include <kernel/vfs/vfs.h>
#include <kernel/fault.h>
#include <kernel/debug.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/types/list.h>

#include <drivers/ide/ide.h>
#include <drivers/fat/fat_access.h>
#include <drivers/fat/fat_table.h>

#include <sys/io_defs.h>
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
	uint8 sfn[11];
	uint32_t cluster;
	list_head_t children;
}dir_info_t;

typedef struct cluster_lookup
{
	uint32_t index;
	uint32_t cluster;
}cluster_lookup_t;

typedef struct file_info
{
	uint32_t start_cluster;
	uint32_t length;
	uint32_t parent_cluster;
	uint8 sfn[11];

	cluster_lookup_t last_fat_lookup;

	// Read/Write sector buffer
	uint8_t file_data_sector[FAT_SECTOR_SIZE];
	uint32_t file_data_address;
	bool file_data_dirty;

}file_info_t;

static partition_fs_t* _mounted_partition = NULL;

static struct fatfs* _get_fatfs(fs_node_t* node)
{
	return &_mounted_partition->fat;
}

/*
VFS interface
*/

fs_node_t* _create_dir_node(const char* name, uint32_t cluster);
fs_node_t* _create_file_node(const char* name, uint32_t length);

static uint32_t _read_sectors(struct fatfs* fatfs, file_info_t* file, uint8_t* buffer, uint32_t offset, uint32_t size)
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
}

static size_t _fs_read_file(fs_node_t* node, uint8_t* buffer, size_t off, size_t count)
{
	ASSERT(node && buffer);
	struct fatfs* fatfs = _get_fatfs(node);
	//Open?
	if (node->data == NULL)
		return 0;
	file_info_t* file = (file_info_t*)node->data;

	//printf("Reading len=%d\n", file->length);

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

	uint32_t bytes_read = 0;
	uint32_t copy_count;
	while (bytes_read < count)
	{
		// Read whole sector, read from media directly into target buffer
		if ((offset == 0) && ((count - bytes_read) >= FAT_SECTOR_SIZE))
		{
			// Read as many sectors as possible into target buffer
			uint32_t sectors_read = _read_sectors(fatfs, file, (uint8_t*)(buffer + bytes_read), sector, (count - bytes_read) / FAT_SECTOR_SIZE);
			if (sectors_read)
			{
				// We have upto one sector to copy
				copy_count = FAT_SECTOR_SIZE * sectors_read;

				// Move onto next sector and reset copy offset
				sector += sectors_read;
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
			if (copy_count > (count - bytes_read))
				copy_count = (count - bytes_read);

			// Copy to application buffer
			memcpy((uint8_t*)((uint8_t*)buffer + bytes_read), (uint8_t*)(file->file_data_sector + offset), copy_count);

			// Move onto next sector and reset copy offset
			sector++;
			offset = 0;
		}

		// Increase total read count
		bytes_read += copy_count;
	}

	return bytes_read;
}

static int _add_free_space(struct fatfs* fs, uint32_t* startCluster, uint32_t clusters)
{
	uint32_t i;
	uint32_t nextcluster;
	uint32_t start = *startCluster;

	// Set the next free cluster hint to unknown
	if (fs->next_free_cluster != FAT32_LAST_CLUSTER)
		fatfs_set_fs_info_next_free_cluster(fs, FAT32_LAST_CLUSTER);

	for (i = 0; i < clusters; i++)
	{
		// Start looking for free clusters from the beginning
		if (fatfs_find_blank_cluster(fs, fs->rootdir_first_cluster, &nextcluster))
		{
			// Point last to this
			fatfs_fat_set_cluster(fs, start, nextcluster);

			// Point this to end of file
			fatfs_fat_set_cluster(fs, nextcluster, FAT32_LAST_CLUSTER);

			// Adjust argument reference
			start = nextcluster;
			if (i == 0)
				*startCluster = nextcluster;
		}
		else
			return 0;
	}

	return 1;
}

static uint32_t _write_sectors(struct fatfs* fatfs, file_info_t* file, uint32_t offset, uint8_t* buf, uint32_t count)
{
	uint32_t sector_num = 0;
	uint32_t cluster_idx = 0;
	uint32_t cluster = 0;
	uint32_t last_cluster = FAT32_LAST_CLUSTER;
	uint32_t i;
	uint32_t lba;
	uint32_t write_count = count;

	// Find values for cluster index & sector within cluster
	cluster_idx = offset / fatfs->sectors_per_cluster;
	sector_num = offset - (cluster_idx * fatfs->sectors_per_cluster);

	// Limit number of sectors written to the number remaining in this cluster
	if ((sector_num + count) > fatfs->sectors_per_cluster)
		count = fatfs->sectors_per_cluster - sector_num;

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
			uint32_t nextCluster;

			// Does the entry exist in the cache?
			//if (!fatfs_cache_get_next_cluster(fatfs, file, i, &nextCluster))
			{
				// Scan file linked list to find next entry
				nextCluster = fatfs_find_next_cluster(fatfs, cluster);

				// Push entry into cache
		//		fatfs_cache_set_next_cluster(fatfs, file, i, nextCluster);
			}

			last_cluster = cluster;
			cluster = nextCluster;

			// Dont keep following a dead end
			if (cluster == FAT32_LAST_CLUSTER)
				break;
		}

		// If we have reached the end of the chain, allocate more!
		if (cluster == FAT32_LAST_CLUSTER)
		{
			// Add some more cluster(s) to the last good cluster chain
			if (!_add_free_space(fatfs, &last_cluster, (write_count + fatfs->sectors_per_cluster - 1) / fatfs->sectors_per_cluster))
				return 0;

			cluster = last_cluster;
		}

		// Record current cluster lookup details
		file->last_fat_lookup.cluster = cluster;
		file->last_fat_lookup.index = cluster_idx;
	}

	// Calculate write address
	lba = fatfs_lba_of_cluster(fatfs, cluster) + sector_num;

	return fatfs_sector_write(fatfs, lba, buf, count);
}

static size_t _fs_write_file(fs_node_t* node, const uint8_t* buffer, size_t file_off, size_t count)
{
	uint32_t sector;
	uint32_t offset;
	uint32_t bytes_written = 0;
	uint32_t copy_count;

	ASSERT(node->data);
	file_info_t* file = (file_info_t*)node->data;
	
	struct fatfs* fatfs = _get_fatfs(node);
	ASSERT(fatfs);

	// No write permissions
	/*if (!(file->flags & FILE_WRITE))
	{
		FL_UNLOCK(&_fs);
		return -1;
	}

	// Append writes to end of file
	if (file->flags & FILE_APPEND)
		file_off = file->filelength;
	// Else write to current position
	*/

	// Calculate start sector
	sector = file_off / FAT_SECTOR_SIZE;

	// Offset to start copying data from first sector
	offset = file_off % FAT_SECTOR_SIZE;

	while (bytes_written < count)
	{
		// Whole sector or more to be written?
		if ((offset == 0) && ((count - bytes_written) >= FAT_SECTOR_SIZE))
		{
			uint32_t sectors_written;

			// Buffered sector, flush back to disk
			if (file->file_data_address != 0xFFFFFFFF)
			{
				// Flush un-written data to file
				//if (file->file_data_dirty)
					//fl_fflush(file);

				file->file_data_address = 0xFFFFFFFF;
				file->file_data_dirty = 0;
			}

			// Write as many sectors as possible
			sectors_written = _write_sectors(fatfs, file, sector, (uint8_t*)(buffer + bytes_written), (count - bytes_written) / FAT_SECTOR_SIZE);
			copy_count = FAT_SECTOR_SIZE * sectors_written;

			// Increase total read count
			bytes_written += copy_count;

			// Increment file pointer
			file_off += copy_count;

			// Move onto next sector and reset copy offset
			sector += sectors_written;
			offset = 0;

			if (!sectors_written)
				break;
		}
		else
		{
			// We have upto one sector to copy
			copy_count = FAT_SECTOR_SIZE - offset;

			// Only require some of this sector?
			if (copy_count > (count - bytes_written))
				copy_count = (count - bytes_written);

			// Do we need to read a new sector?
			if (file->file_data_address != sector)
			{
				// Flush un-written data to file
			//	if (file->file_data_dirty)
				//	fl_fflush(file);

				// If we plan to overwrite the whole sector, we don't need to read it first!
				if (copy_count != FAT_SECTOR_SIZE)
				{
					// NOTE: This does not have succeed; if last sector of file
					// reached, no valid data will be read in, but write will
					// allocate some more space for new data.

					// Get LBA of sector offset within file
					if (!_read_sectors(fatfs, file, file->file_data_sector, sector, 1))
						memset(file->file_data_sector, 0x00, FAT_SECTOR_SIZE);
				}

				file->file_data_address = sector;
				file->file_data_dirty = 0;
			}

			// Copy from application buffer into sector buffer
			memcpy((uint8_t*)(file->file_data_sector + offset), (uint8_t*)(buffer + bytes_written), copy_count);

			// Mark buffer as dirty
			//file->file_data_dirty = 1;

			if (_write_sectors(fatfs, file, file->file_data_address, file->file_data_sector, 1) != 1)
			{
				ASSERT(false);
			}
			//printf("Writing sector offset %d %c %c\n", file->file_data_address, file->file_data_sector[0], file->file_data_sector[1]);

			// Increase total read count
			bytes_written += copy_count;

			// Increment file pointer
			file_off += copy_count;
			
			// Move onto next sector and reset copy offset
			sector++;
			offset = 0;
		}
	}

	// Write increased extent of the file?
	if (file_off > file->length)
	{
		// Increase file size to new point
		file->length = file_off;

		// We are changing the file count and this
		// will need to be writen back at some point
		//file->filelength_changed = 1;
		fatfs_update_file_length(fatfs, file->parent_cluster, node->name, file->length);
	}

#if FATFS_INC_TIME_DATE_SUPPORT
	// If time & date support is enabled, always force directory entry to be
	// written in-order to update file modify / access time & date.
	file->filelength_changed = 1;
#endif

	return bytes_written;
}

static uint32_t _get_entry_start_cluster(struct fat_dir_entry* entry)
{
	return ((FAT_HTONS((uint32_t)entry->FstClusHI)) << 16) + FAT_HTONS(entry->FstClusLO);
}

static bool _fs_open_file(fs_node_t* parent, fs_node_t* node)
{
	ASSERT(fs_is_dir(parent) && !fs_is_dir(node));
	dir_info_t* dir = (dir_info_t*)parent->data;
	struct fatfs* fatfs = _get_fatfs(node);

	if (node->data)
		return true; //open

	//printf("FAT openeing %s\n", node->name);

	struct fat_dir_entry sfEntry;
	if (fatfs_get_file_entry(fatfs, dir->cluster, node->name, &sfEntry))
	{
		file_info_t* file = (file_info_t*)kmalloc(sizeof(file_info_t));
		file->start_cluster = _get_entry_start_cluster(&sfEntry);
		file->length = FAT_HTONL(sfEntry.FileSize);
		file->parent_cluster = dir->cluster;
		strcpy(file->sfn, sfEntry.Name); //save the short file name for faster lookup
		
		file->last_fat_lookup.index = 0xFFFFFFFF;
		file->last_fat_lookup.cluster = 0xFFFFFFFF;

		file->file_data_address = 0xFFFFFFFF;
		file->file_data_dirty = 0;

		node->data = file;
		return true;
	}
	return false;
}

static void _fs_close_file(fs_node_t* node)
{
	//printf("Closing file %s\n", node->name);
	if (!node->data)
		return;

	file_info_t* file = (file_info_t*)node->data;
	kfree(file);
	node->data = NULL;
}

static bool _fs_remove(fs_node_t* parent, fs_node_t* node)
{
	ASSERT(node);
	ASSERT(parent);
	ASSERT(parent->data);
	ASSERT(fs_is_dir(parent));
	struct fatfs* fatfs = _get_fatfs(node);
	dir_info_t* dir = (dir_info_t*)parent->data;

	//printf("Removing %s\n", node->name);

	struct fat_dir_entry sfEntry;
	if (fatfs_get_file_entry(fatfs, dir->cluster, node->name, &sfEntry))
	{
		uint32_t start_cluster = _get_entry_start_cluster(&sfEntry);
		if (fatfs_free_cluster_chain(fatfs, start_cluster))
		{
			if (fatfs_mark_file_deleted(fatfs, dir->cluster, sfEntry.Name))
			{
				list_delete(&node->list);
				return true;
			}
		}
	}
	return false;
}

static void _populate_dir_children(fs_node_t* dir_node)
{
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
		//	printf("FAT Populate Adding DIR %s \n", dirent.filename);
			entry = _create_dir_node(dirent.filename, dirent.cluster);
		}
		else
		{
		//	printf("FAT Populate Adding FILE %s \n", dirent.filename);
			entry = _create_file_node(dirent.filename, dirent.size);
			
		}
		list_add(&entry->list, &info->children);
	}
}

static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
//	printf("FAT32 read dir %s\n", node->name);
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
	//printf("FAT32 find %s in %s\n", name, node->name);
	dir_info_t* dir = (dir_info_t*)node->data;
	if (list_empty(&dir->children))
		_populate_dir_children(node);

	fs_node_t* child;
	list_for_each_entry(child, &dir->children, list)
		if (strcmp(name, child->name) == 0)
			return child;
	return NULL;
}

static fs_node_t* _fs_create_file(fs_node_t* parent, const char* name)
{
	return NULL;
}

static fs_node_t* _fs_create_child(fs_node_t* parent, const char* name, uint32_t flags)
{
	if (flags & FS_FILE)
	{
		return _fs_create_file(parent, name);
	}
	return NULL;
}

fs_node_t* _create_file_node(const char* name, uint32_t length)
{
	fs_node_t* node = fs_create_node(name);
	node->len = length;
	node->read = _fs_read_file;
	node->open = _fs_open_file;
	node->write = _fs_write_file;
	node->close = _fs_close_file;
	node->remove = _fs_remove;
	return node;
}

fs_node_t* _create_dir_node(const char* name, uint32_t cluster)
{
	fs_node_t* node = fs_create_node(name);
	node->inode = 0;
	node->flags |= FS_DIR;
	node->read_dir = _fs_read_dir;
	node->find_child = _fs_find_child;
	node->create_child = _fs_create_child;
	node->remove = _fs_remove;	
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
	//printf("Writing sector %d %c %c\n", sector, buffer[0], buffer[1]);
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

	//printf("Mounting partition %d lba= 0x%x\n", part, _mounted_partition->part_lba);

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