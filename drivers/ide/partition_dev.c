#include "partition_dev.h"

#include <drivers/device_driver.h>
#include <drivers/ide/ide.h>

#include <stdbool.h>

//need block device
# if 0
typedef struct partition_dev
{
	ide_device_t* ide;
	uint32_t start_lba;

}partition_dev_t;

static bool _dev_registered = false;
static dev_driver_t _driver;

static size_t _read_partition(dev_device_t* dev, uint8_t buff, size_t off, size_t sz)
{

}

static size_t _write_partition(dev_device_t* dev, const uint8_t* buff, size_t off, size_t sz)
{

}

static void _register_dev()
{
	memset(&_driver, 0, sizeof(dev_driver_t));
	kname_set("hdd_driver", &_driver.name);
	_driver.read = &_read_partition;
	_driver.write = &_write_partition;
	dev_install_driver(&_driver);

	_dev_registered = true;
}

void partition_register(ide_device_t* ide, uint8_t drive, uint8_t part)
{
	if (!_dev_registered)
		_register_dev();

	ASSERT(ide->type == IDE_ATA);

	ide_partition_t* partition = &ide->partitions[part];
	if (!partition->present)
	{
		ASSERT(ide);
		return;
	}

	
}

#endif