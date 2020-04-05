#include "devfs.h"

#include <kernel/fs/fs.h>
#include <kernel/fs/dir.h>
#include <kernel/fault.h>

#include <stdio.h>

static fs_node_t* _root;

static size_t _write_device(fs_node_t* f, const uint8_t* buff, size_t off, size_t sz)
{
	dev_device_t* device = (dev_device_t*)f->data;
	return device->driver->write(device, buff, off, sz);
}

static size_t _read_device(fs_node_t* f, uint8_t* buff, size_t off, size_t sz)
{
	ASSERT(f && buff && sz);
	dev_device_t* device = (dev_device_t*)f->data;
	return device->driver->read(device, buff, off, sz);
}

static int32_t _open_device(fs_node_t* f, uint32_t flags)
{
	ASSERT(f);
	dev_device_t* device = (dev_device_t*)f->data;
	return device->driver->open(device, flags);
}

static void _close_device(fs_node_t* f)
{
	ASSERT(f);
	dev_device_t* device = (dev_device_t*)f->data;
	return device->driver->close(device);
}

void devfs_init()
{
	_root = fs_create_dir_node("dev", 0);
	fs_install_root_fs(_root);
}

void devfs_register_device(dev_device_t* device)
{
	//printf("Dev reg 0x%08x %s\n", device->driver, device->name.str);

	device->fs_node = fs_create_node(device->name.str);
	device->fs_node->data = device;
	if(device->driver->read)
		device->fs_node->read = &_read_device;
	if (device->driver->write)
		device->fs_node->write = &_write_device;
	if (device->driver->open)
		device->fs_node->open = &_open_device;
	if (device->driver->close)
		device->fs_node->close = &_close_device;

	fs_node_t* parent = _root;
	if (device->driver->device_subdir.str)
	{
		//device wants a subdir, create if required
		parent = fs_find_child(_root, device->driver->device_subdir.str);
		if (!parent)
			parent = fs_add_child_node(_root, fs_create_dir_node(device->driver->device_subdir.str, 0));
	}

	if (!fs_add_child_node(parent, device->fs_node))
	{
		fs_destroy_node(device->fs_node);
		return;
	}
}