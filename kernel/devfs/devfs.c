#include "devfs.h"

#include <kernel/vfs/vfs.h>
#include <kernel/fault.h>
#include <kernel/types/list.h>

#include <stdio.h>

static fs_node_t* _devfs_root;
static list_head_t _dev_list;

static fs_node_t* _dev_con = 0;

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

static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	ASSERT(node == _devfs_root);
	if(_dev_con)
		cb(node, _dev_con, data);

	return _dev_con ? 1 : 0;
}

static fs_node_t* _fs_find_child(fs_node_t* node, const char* name)
{
	return _dev_con;
}

void devfs_init()
{
	INIT_LIST_HEAD(&_dev_list);

	_devfs_root = fs_create_node("dev");
	_devfs_root->inode = 0;
	_devfs_root->flags |= FS_DIR;
	_devfs_root->read_dir = _fs_read_dir;
	_devfs_root->find_child = _fs_find_child;

	fs_install_root_fs(_devfs_root);
}

void devfs_register_device(dev_device_t* device)
{
	list_add(&device->list, &_dev_list);
	
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

	_dev_con = device->fs_node;


	
}