#include "ata.h"
#include <drivers/ioports.h>
#include <drivers/console.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <stdio.h>

//Channels
#define ATA_PRIMARY			0x00
#define ATA_SECONDARY		0x01


#define ATA_MASTER			0x00
#define ATA_SLAVE			0x01

#define ATA_PRIMARY_IO		0x1F0
#define ATA_SECONDARY_IO	0x170

// ATA-ATAPI Task-File
#define ATA_REG_DATA		0x00
#define ATA_REG_ERROR		0x01
#define ATA_REG_FEATURES	0x01
#define ATA_REG_SECCOUNT0   0x02
#define ATA_REG_LBA0		0x03
#define ATA_REG_LBA1		0x04
#define ATA_REG_LBA2		0x05
#define ATA_REG_HDDEVSEL	0x06
#define ATA_REG_COMMAND     0x07
#define ATA_REG_STATUS      0x07
#define ATA_REG_SECCOUNT1   0x08
#define ATA_REG_LBA3		0x09
#define ATA_REG_LBA4		0x0A
#define ATA_REG_LBA5		0x0B
#define ATA_REG_CONTROL     0x0C
#define ATA_REG_ALTSTATUS   0x0C
#define ATA_REG_DEVADDRESS  0x0D


#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

//Offsets into the 512 byte structure return by the IDENTIFY command
#define ATA_IDENT_DEVICETYPE	0
#define ATA_IDENT_CYLINDERS		2
#define ATA_IDENT_HEADS			6
#define ATA_IDENT_SECTORS		12
#define ATA_IDENT_SERIAL		20
#define ATA_IDENT_MODEL			54
#define ATA_IDENT_CAPABILITIES  98
#define ATA_IDENT_FIELDVALID	106
#define ATA_IDENT_MAX_LBA		120
#define ATA_IDENT_COMMANDSETS   164
#define ATA_IDENT_MAX_LBA_EXT   200

uint8_t ide_buf[512];

void ide_400ns_delay(uint16_t io)
{
	for (int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

uint8_t ide_poll(uint16_t io)
{
	ide_400ns_delay(io);
	uint8_t status;
	do
	{
		status = inb(io + ATA_REG_STATUS);
	} while (status & ATA_SR_BSY);

	status = inb(io + ATA_REG_STATUS);
	if (status & ATA_SR_ERR)
		return 0; //error
	if (status & ATA_SR_DF)
		return 0; //device failure
	if (!(status & ATA_SR_DRQ))
		return 0; //DRQ must be set //todo loop?
	return 1;
}

void ata_detect()
{
	//select primary master
	outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);

	uint16_t base_port = ATA_PRIMARY_IO;
	/* ATA specs say these values must be zero before sending IDENTIFY */
	outb(base_port + ATA_REG_SECCOUNT0, 0);
	outb(base_port + ATA_REG_LBA0, 0);
	outb(base_port + ATA_REG_LBA1, 0);
	outb(base_port + ATA_REG_LBA2, 0);
	outb(base_port + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	printf("Sent IDENTIFY\n");
	uint8_t status = inb(base_port + ATA_REG_STATUS);

	if (status)
	{
		if (!ide_poll(base_port))
		{
			printf("Disk Error\n");
			return;
		}
	//	printf("Disk online\n");
		for (int i = 0; i<256; i++)
		{
			*(uint16_t *)(ide_buf + i * 2) = inw(base_port + ATA_REG_DATA);
		}

		char *str = (char *)kmalloc(40);
		for (int i = 0; i < 40; i += 2)
		{
			str[i] = ide_buf[ATA_IDENT_MODEL + i + 1];
			str[i + 1] = ide_buf[ATA_IDENT_MODEL + i];
		}
		str[39] = '\0';

		printf("Disk is %s\n", str);
	}
}

uint8_t ata_read_one(uint8_t *buf, uint32_t lba)
{
	//28bit LBA
	uint8_t drive;
	uint16_t io = 0;
	io = ATA_PRIMARY_IO;
	drive = ATA_MASTER;
	
	uint8_t cmd = (drive == ATA_MASTER ? 0xE0 : 0xF0);
	outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	outb(io + 1, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	if (!ide_poll(io))
		return 0;
	for (int i = 0; i < 256; i++)
	{
		uint16_t data = inw(io + ATA_REG_DATA);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide_400ns_delay(io);
	return 1;
}

void ata_read(uint8_t *buf, uint32_t lba, uint32_t numsects)
{
	for (int i = 0; i < numsects; i++)
	{
		ata_read_one(buf, lba + i);
		buf += 512;
	}
}

uint8_t buffer[1024];

void ata_init()
{
	ata_detect();
	ata_read(buffer, 0, 2);

	//printf("hdd read: 0x%x 0x%x\n", buffer[1022], buffer[1023]);

}