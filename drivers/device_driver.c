#include "device_driver.h"

#include <kernel/devfs/devfs.h>

static list_head_t _driver_list;

void dev_init()
{
	INIT_LIST_HEAD(&_driver_list);
}

void dev_install_driver(dev_driver_t* driver)
{
	list_add(&driver->list, &_driver_list);
}

void dev_register_device(dev_device_t* device)
{
	devfs_register_device(device);
}