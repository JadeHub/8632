#pragma once

#include <kernel/types/list.h>
#include <kernel/types/kname.h>
#include <kernel/vfs/node.h>
#include <kernel/tasks/proc.h>

#include <stdint.h>

struct dev_driver;

typedef struct device
{
	kname_t name;
	fs_node_t* fs_node;
	struct dev_driver* driver;

	thread_t* read_waiters;
}dev_device_t;

typedef size_t (*dev_read_fs)(dev_device_t*, uint8_t* buff, size_t off, size_t sz);
typedef size_t (*dev_write_fs)(dev_device_t*, const uint8_t* buff, size_t off, size_t sz);
typedef int32_t (*dev_open_fs)(dev_device_t*, uint32_t);
typedef void (*dev_close_fs)(dev_device_t*);

typedef struct dev_driver
{
	kname_t name;
	kname_t device_subdir;
	list_head_t list;
	dev_write_fs write;
	dev_read_fs read;
	dev_open_fs open;
	dev_close_fs close;
}dev_driver_t;

void dev_init();
void dev_install_driver(dev_driver_t* driver);
void dev_register_device(dev_device_t* device);

void dev_block_until_read(dev_device_t*);
void dev_unblock_readers(dev_device_t*);