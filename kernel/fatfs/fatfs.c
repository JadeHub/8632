#include "fatfs.h"

#include <kernel/vfs/vfs.h>

#include <drivers/fat/fat_access.h>

#include <stdint.h>

static size_t _read_file(fs_node_t* node, uint8_t* buff, size_t off, size_t sz)
{
	return 0;
}

static size_t _write_file(fs_node_t* node, uint8_t* buff, size_t off, size_t sz)
{
	return 0;
}

static int32_t _open_file(fs_node_t* node, uint32_t flags)
{
	return 0;
}

static void _close_file(fs_node_t* node)
{
}

static uint32_t _read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	return 0;
}

void fatfs_mount_partition(uint8_t ide_controller, uint8_t drive, uint8_t partition)
{


}