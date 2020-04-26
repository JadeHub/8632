#pragma once

#include <stdint.h>
#include <stdbool.h>

// Channels:
#define ATA_PRIMARY      0x00
#define ATA_SECONDARY    0x01

// ATA Registers
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

#define IDE_ATA 0
#define IDE_ATAPI 1

#define IDE_ERR_NONE		0
#define IDE_ERR_DEV			1
#define IDE_ERR_DRQ			2

typedef struct
{
	bool present;
	uint32_t lba;
	uint32_t sectors;
	uint8_t type;
}ide_partition_t;

typedef struct
{
	bool present;
	struct ide_channel_regs* channel;
	uint8_t drive; //0 (Master) or 1 (Slave)
	uint8_t type; //0 (ATA) or 1 (ATAPI)
	uint16_t signature;
	uint16_t capabilities;
	uint32_t cmd_sets;
	uint32_t sectors;
	char model[41];
	ide_partition_t partitions[4];
}ide_device_t;

void ide_init(uint8_t bus, uint8_t slot);
uint8_t ide_read_sectors(ide_device_t* ide, uint8_t numsects, uint32_t lba, void* buff);
uint8_t ide_write_sectors(ide_device_t* ide, uint8_t numsects, uint32_t lba, void* buff);
ide_device_t* ide_get_device(uint8_t controller, uint8_t drive);