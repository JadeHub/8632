#include "fatfs1.h"

#include <drivers/fat/fat_filelib.h>
#include <drivers/pci/ide.h>

uint32_t _offset = 0;
ide_device_t* _ide = 0;

static int _fl_disk_read(unsigned long sector, unsigned char* buffer, unsigned long sector_count)
{
	ide_read_sectors(_ide, sector_count, _offset + sector, buffer);
	return 1;
}

static int _fl_disk_write(unsigned long sector, unsigned char* buffer, unsigned long sector_count)
{
	ide_write_sectors(_ide, sector_count, _offset + sector, buffer);
	return 1;
}

void fatfs1_mount_partition(uint8_t ide_controller, uint8_t drive, uint8_t part)
{
	ide_device_t* ide = ide_get_device(ide_controller, drive);
	if (!ide)
		return;

	ide_partition_t* partition = &ide->partitions[part];
	if (!partition->present)
		return;

	printf("Mounting partition at offset 0x%x\n", partition->lba);

	_offset = partition->lba;
	_ide = ide;

	fl_init();
	if (fl_attach_media(_fl_disk_read, _fl_disk_write) != FAT_INIT_OK)
	{
		printf("disk: Cannot init FAT32\n");
		_offset = -1;
	}
	printf("fatfs init\n");

	char buff[120];
	void* file = fl_fopen("/test.txt", "r");
	fl_fread(buff, 5, 1, file);
	fl_fclose(file);
	buff[5] = '\0';
	printf("Read %s\n", buff);

	//fl_listdirectory("/");
	fl_listdirectory("/initrd/bin");
}