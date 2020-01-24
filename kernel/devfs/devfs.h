#pragma once

#include <drivers/device_driver.h>

void devfs_init();
void devfs_register_device(dev_device_t* device);