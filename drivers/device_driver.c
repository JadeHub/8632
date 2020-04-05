#include "device_driver.h"

#include <kernel/devfs/devfs.h>
#include <kernel/tasks/sched.h>

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

void dev_block_until_read(dev_device_t* device)
{
	sched_cur_thread()->next = device->read_waiters;
	device->read_waiters = sched_cur_thread();
	sched_block();
}

void dev_unblock_readers(dev_device_t* device)
{
	thread_t* thread = device->read_waiters;
	device->read_waiters = NULL;
	while (thread)
	{
		sched_unblock(thread);
		thread = thread->next;
	}
}