#include "ide.h"
#include "mbr.h"

#include <drivers/pci/pci.h>
#include <drivers/ioports.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/fault.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//ATA STATUS
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Inlex
#define ATA_SR_ERR     0x01    // Error


//ATA COMMANDS
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
#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

//IDENTs
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define ATA_READ 0
#define ATA_WRITE 1

typedef struct ide_channel_regs
{
	uint8_t index;
	uint16_t base;  // I/O Base.
	uint16_t ctrl;  // Control Base
	uint16_t bmide; // Bus Master IDE
	uint8_t  nIEN;  // nIEN (No Interrupt);
} ide_channel_regs_t;

typedef struct
{
	uint8_t bus;
	uint8_t slot;
	ide_device_t devices[4];
	ide_channel_regs_t channels[2];
}ide_ctrl_t;

static ide_ctrl_t* _ide_controllers[2] = { NULL, NULL };

static void _detect_partitions(ide_device_t* ide)
{
	uint8_t mbr_buff[512];
	memset(mbr_buff, 0, 512);

	ide_read_sectors(ide, 1, 0, mbr_buff);
	mbr_t* mbr = (mbr_t*)mbr_buff;

	if (mbr->signature != 0xAA55)
		return;

	for (int i = 0; i < 4; i++)
	{
		ide->partitions[i].present = mbr->partitions[i].type != 0 && mbr->partitions[i].type != 0xEE;
		ide->partitions[i].type = mbr->partitions[i].type;
		ide->partitions[i].lba = mbr->partitions[i].lba;
		ide->partitions[i].sectors = mbr->partitions[i].sectors;

		printf("Partition type: 0x%x, lba: 0x%x, size: 0x%x present %d\n",
			ide->partitions[i].type,
			ide->partitions[i].lba,
			ide->partitions[i].sectors,
			ide->partitions[i].present);
		/*if (mbr->partitions[i].type == 0xee)
		{
			printf("Protective MBR found, assuming GPT partition table\n");
			//ide_detect_gpt_partitions();
			return;
		}
		if (mbr->partitions[i].type == 0xc)
		{
			printf("Partition found %d at LBA %d (size: %d) FAT32\n", i, mbr->partitions[i].lba, mbr->partitions[i].sectors);

		}*/
	}
}

static inline void insl(int port, void* addr, int cnt)
{
	asm volatile("cld; rep insl" :
	"=D" (addr), "=c" (cnt) :
		"d" (port), "0" (addr), "1" (cnt) :
		"memory", "cc");
}

static inline void outsw(uint16_t port, const void* addr, int cnt)
{
	asm volatile("rep; outsw" : "+S" (addr), "+c" (cnt) : "d" (port));
}

static inline void insw(uint16_t port, void* addr, int cnt)
{
	asm volatile("rep; insw"
		: "+D" (addr), "+c" (cnt)
		: "d" (port)
		: "memory");
}

static void _ide_reg_write(ide_channel_regs_t* channel, uint8_t reg, uint8_t data)
{
	if (reg > 0x07 && reg < 0x0C)
		_ide_reg_write(channel, ATA_REG_CONTROL, 0x80 | channel->nIEN);
	if (reg < 0x08)
		outb(channel->base + reg - 0x00, data);
	else if (reg < 0x0C)
		outb(channel->base + reg - 0x06, data);
	else if (reg < 0x0E)
		outb(channel->ctrl + reg - 0x0A, data);
	else if (reg < 0x16)
		outb(channel->bmide + reg - 0x0E, data);
	if (reg > 0x07 && reg < 0x0C)
		_ide_reg_write(channel, ATA_REG_CONTROL, channel->nIEN);
}

static uint8_t _ide_reg_read(ide_channel_regs_t* channel, uint8_t reg)
{
	uint8_t result = 0;
	if (reg > 0x07 && reg < 0x0C)
		_ide_reg_write(channel, ATA_REG_CONTROL, channel->nIEN | 0x80);
	if (reg < 0x08)
		result = inb(channel->base + reg - 0x00);
	else if (reg < 0x0C)
		result = inb(channel->base + reg - 0x06);
	else if (reg < 0x0E)
		result = inb(channel->ctrl + reg - 0x0A);
	else if (reg < 0x16)
		result = inb(channel->bmide + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C)
		_ide_reg_write(channel, ATA_REG_CONTROL, channel->nIEN);
	return result;
}

static void _ide_read_buffer(ide_channel_regs_t* channel, uint8_t reg, void* buffer, uint32_t count)
{
	if (reg > 0x07 && reg < 0x0C)
		_ide_reg_write(channel, ATA_REG_CONTROL, channel->nIEN | 0x80);

	if (reg < 0x08)
		insl(channel->base + reg - 0x00, buffer, count);
	else if (reg < 0x0C)
		insl(channel->base + reg - 0x06, buffer, count);
	else if (reg < 0x0E)
		insl(channel->ctrl + reg - 0x0A, buffer, count);
	else if (reg < 0x16)
		insl(channel->bmide + reg - 0x0E, buffer, count);

	if (reg > 0x07 && reg < 0x0C)
		_ide_reg_write(channel, ATA_REG_CONTROL, channel->nIEN);
}

static uint8_t _ide_polling(ide_channel_regs_t* channel, uint32_t advanced_check)
{
	// Reading the Alternate Status port wastes 100ns; loop four times.
	for (int i = 0; i < 4; i++)
		_ide_reg_read(channel, ATA_REG_ALTSTATUS);

	//Wait for BSY to be cleared
	while (_ide_reg_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);

	if (advanced_check)
	{
		uint8_t state = _ide_reg_read(channel, ATA_REG_STATUS); // Read Status Register.

		if (state & ATA_SR_ERR)
			return 2; // Error.
		if (state & ATA_SR_DF)
			return 1; // Device Fault.
		if ((state & ATA_SR_DRQ) == 0)
			return 3; // DRQ should be set
	}
	return 0;
}

/*static uint8_t ide_atapi_read(ide_device_t* ide, uint32_t lba, uint8_t numsects, uint16_t selector, uint32_t edi)
{
	uint32_t   channel = ide_devices[drive].channel;
	uint32_t   slavebit = ide_devices[drive].drive;
	uint32_t   bus = ide->channel->base;
	uint32_t   words = 2048 / 2; // Sector Size in Words, Almost All ATAPI Drives has a sector size of 2048 bytes.
	uint8_t  err; int i;
	// Enable IRQs:
	ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = ide_irq_invoked = 0x0);
	// (I): Setup SCSI Packet:
	// ------------------------------------------------------------------
	atapi_packet[0] = ATAPI_CMD_READ;
	atapi_packet[1] = 0x0;
	atapi_packet[2] = (lba >> 24) & 0xFF;
	atapi_packet[3] = (lba >> 16) & 0xFF;
	atapi_packet[4] = (lba >> 8) & 0xFF;
	atapi_packet[5] = (lba >> 0) & 0xFF;
	atapi_packet[6] = 0x0;
	atapi_packet[7] = 0x0;
	atapi_packet[8] = 0x0;
	atapi_packet[9] = numsects;
	atapi_packet[10] = 0x0;
	atapi_packet[11] = 0x0;
	// (II): Select the Drive:
	// ------------------------------------------------------------------
	ide_write(channel, ATA_REG_HDDEVSEL, slavebit << 4);
	// (III): Delay 400 nanosecond for select to complete:
	// ------------------------------------------------------------------
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
	// (IV): Inform the Controller that we use PIO mode:
	// ------------------------------------------------------------------
	ide_write(channel, ATA_REG_FEATURES, 0);         // PIO mode.
	// (V): Tell the Controller the size of buffer:
	// ------------------------------------------------------------------
	ide_write(channel, ATA_REG_LBA1, (words * 2) & 0xFF);   // Lower Byte of Sector Size.
	ide_write(channel, ATA_REG_LBA2, (words * 2) >> 8);   // Upper Byte of Sector Size.
	// (VI): Send the Packet Command:
	// ------------------------------------------------------------------
	ide_write(channel, ATA_REG_COMMAND, ATA_CMD_PACKET);      // Send the Command.
	// (VII): Waiting for the driver to finish or invoke an error:
	// ------------------------------------------------------------------
	if (err = ide_polling(channel, 1)) return err;         // Polling and return if error.
   // (VIII): Sending the packet data:
	// ------------------------------------------------------------------
	asm("rep   outsw"::"c"(6), "d"(bus), "S"(atapi_packet));   // Send Packet Data
	// (IX): Recieving Data:
	// ------------------------------------------------------------------
	for (i = 0; i < numsects; i++) {
		ide_wait_irq();                  // Wait for an IRQ.
		if (err = ide_polling(channel, 1)) return err;      // Polling and return if error.
		asm("pushw %es");
		asm("mov %%ax, %%es"::"a"(selector));
		asm("rep insw"::"c"(words), "d"(bus), "D"(edi));// Receive Data.
		asm("popw %es");
		edi += (words * 2);
	}
	// (X): Waiting for an IRQ:
	// ------------------------------------------------------------------
	ide_wait_irq();

	// (XI): Waiting for BSY & DRQ to clear:
	// ------------------------------------------------------------------
	while (ide_read(channel, ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ));

	return 0; // Easy, ... Isn't it?
}*/

static uint8_t _ide_ata_access(ide_device_t* ide, uint8_t direction, uint32_t lba, uint8_t numsects, void* buff) 
{
	uint8_t lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, dma /* 0: No DMA, 1: DMA */, cmd;
	uint8_t lba_io[6];
	
	uint32_t slavebit = ide->drive; // Read the Drive [Master/Slave]
	uint32_t bus = ide->channel->base; // Bus Base, like 0x1F0 which is also data port.
	uint32_t words = 256;
	uint16_t cyl, i;
	uint8_t head, sect, err;

	_ide_reg_write(ide->channel, ATA_REG_CONTROL, ide->channel->nIEN = 0x02);
	// Select one from LBA28, LBA48 or CHS;
	if (lba >= 0x10000000)
	{
	   // LBA48:
		lba_mode = 2;
		lba_io[0] = (lba & 0x000000FF) >> 0;
		lba_io[1] = (lba & 0x0000FF00) >> 8;
		lba_io[2] = (lba & 0x00FF0000) >> 16;
		lba_io[3] = (lba & 0xFF000000) >> 24;
		lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
		lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
		head = 0; // Lower 4-bits of HDDEVSEL are not used here.
	}
	else if (ide->capabilities & 0x200)
	{
		// LBA28:
		lba_mode = 1;
		lba_io[0] = (lba & 0x00000FF) >> 0;
		lba_io[1] = (lba & 0x000FF00) >> 8;
		lba_io[2] = (lba & 0x0FF0000) >> 16;
		lba_io[3] = 0; // These Registers are not used here.
		lba_io[4] = 0; // These Registers are not used here.
		lba_io[5] = 0; // These Registers are not used here.
		head = (lba & 0xF000000) >> 24;
	}
	else
	{
		// CHS:
		lba_mode = 0;
		sect = (lba % 63) + 1;
		cyl = (lba + 1 - sect) / (16 * 63);
		lba_io[0] = sect;
		lba_io[1] = (cyl >> 0) & 0xFF;
		lba_io[2] = (cyl >> 8) & 0xFF;
		lba_io[3] = 0;
		lba_io[4] = 0;
		lba_io[5] = 0;
		head = (lba + 1 - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
	}
	dma = 0;
	while (_ide_reg_read(ide->channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait if busy.

	if (lba_mode == 0)
		_ide_reg_write(ide->channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head); // Drive & CHS.
	else
		_ide_reg_write(ide->channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head); // Drive & LBA
	if (lba_mode == 2)
	{
		_ide_reg_write(ide->channel, ATA_REG_SECCOUNT1, 0);
		_ide_reg_write(ide->channel, ATA_REG_LBA3, lba_io[3]);
		_ide_reg_write(ide->channel, ATA_REG_LBA4, lba_io[4]);
		_ide_reg_write(ide->channel, ATA_REG_LBA5, lba_io[5]);
	}
	_ide_reg_write(ide->channel, ATA_REG_SECCOUNT0, numsects);
	_ide_reg_write(ide->channel, ATA_REG_LBA0, lba_io[0]);
	_ide_reg_write(ide->channel, ATA_REG_LBA1, lba_io[1]);
	_ide_reg_write(ide->channel, ATA_REG_LBA2, lba_io[2]);
	if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
	if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
	if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;
	if (lba_mode == 0 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
	if (lba_mode == 1 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
	if (lba_mode == 2 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA_EXT;
	if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
	if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
	if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
	if (lba_mode == 0 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
	if (lba_mode == 1 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
	if (lba_mode == 2 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
	//send the Command
	_ide_reg_write(ide->channel, ATA_REG_COMMAND, cmd);
	if (dma)
	{
		numsects = 0;
		/*if (direction == ATA_READ);
		// DMA Read.
		else;
		// DMA Write.*/
	}
	else
	{
		// PIO
		if (direction == ATA_READ)
		{
			for (i = 0; i < numsects; i++)
			{
				if ((err = _ide_polling(ide->channel, true)))
					return err;
				insw(bus, (void*)buff, words);
				buff += (words * 2);
			}
		}
		else
		{
			for (i = 0; i < numsects; i++)
			{
				if ((err = _ide_polling(ide->channel, false)))
					return err;
				outsw(bus, (void*)buff, words);
				buff += (words * 2);
			}
			_ide_reg_write(ide->channel, ATA_REG_COMMAND, (char[]) { ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT }[lba_mode]);
			_ide_polling(ide->channel, 0); // Polling.
		}
	}
	return numsects;
}

void ide_init(uint8_t bus, uint8_t slot)
{
	printf("Initialising IDE controller at bus %d slot %d\n", bus, slot);
	ide_ctrl_t* ide = (ide_ctrl_t*)kmalloc(sizeof(ide_ctrl_t));
	memset(ide, 0, sizeof(ide_ctrl_t));

	//Get BAR values
	uint32_t bar0, bar1, bar2, bar3, bar4;
	bar0 = pci_config_read_dword(bus, slot, 0, 0x10);
	bar1 = pci_config_read_dword(bus, slot, 0, 0x14);
	bar2 = pci_config_read_dword(bus, slot, 0, 0x18);
	bar3 = pci_config_read_dword(bus, slot, 0, 0x1C);
	bar4 = pci_config_read_dword(bus, slot, 0, 0x20);

	//Set I/O ports
	ide->channels[ATA_PRIMARY].index = 0;
	ide->channels[ATA_PRIMARY].base = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
	ide->channels[ATA_PRIMARY].ctrl = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
	ide->channels[ATA_SECONDARY].index = 0;
	ide->channels[ATA_SECONDARY].base = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
	ide->channels[ATA_SECONDARY].ctrl = (bar3 & 0xFFFFFFFC) + 0x376 * (!bar3);
	ide->channels[ATA_PRIMARY].bmide = (bar4 & 0xFFFFFFFC) + 0; // Bus Master IDE
	ide->channels[ATA_SECONDARY].bmide = (bar4 & 0xFFFFFFFC) + 8; // Bus Master IDE

	_ide_reg_write(&ide->channels[ATA_PRIMARY], ATA_REG_CONTROL, ide->channels[ATA_PRIMARY].nIEN = 0x02);
	_ide_reg_write(&ide->channels[ATA_SECONDARY], ATA_REG_CONTROL, ide->channels[ATA_SECONDARY].nIEN = 0x02);

	int count = 0;
	uint8_t ide_buf[2048];
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			uint8_t err = 0, type = IDE_ATA, status;
			// Select Drive
			_ide_reg_write(&ide->channels[i], ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
			//sleep

			_ide_reg_write(&ide->channels[i], ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			//sleep

			if (_ide_reg_read(&ide->channels[i], ATA_REG_STATUS) == 0)
			{
				count++;
				continue; // If Status = 0, No Device.
			}

			while (1)
			{
				status = _ide_reg_read(&ide->channels[i], ATA_REG_STATUS);
				if ((status & ATA_SR_ERR)) { err = 1; break; } // If Err, Device is not ATA.
				if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
			}

			//Probe for ATAPI Devices:
			if (err != 0)
			{
				uint8_t cl = _ide_reg_read(&ide->channels[i], ATA_REG_LBA1);
				uint8_t ch = _ide_reg_read(&ide->channels[i], ATA_REG_LBA2);

				if (cl == 0x14 && ch == 0xEB)
					type = IDE_ATAPI;
				else if (cl == 0x69 && ch == 0x96)
					type = IDE_ATAPI;
				else
					continue; // Unknown type (may not be a device).

				_ide_reg_write(&ide->channels[i], ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
				//sleep
			}

			//Read Identification Space of the Device:
			memset(ide_buf, 0, 2048);
			_ide_read_buffer(&ide->channels[i], ATA_REG_DATA, ide_buf, 128);
			
			uint16_t* signature = (uint16_t*)(ide_buf + ATA_IDENT_DEVICETYPE);
			uint16_t* capabilities = (uint16_t*)(ide_buf + ATA_IDENT_CAPABILITIES);
			uint32_t* cmdsets = (uint32_t*)(ide_buf + ATA_IDENT_COMMANDSETS);
			ide->devices[count].present = true;
			ide->devices[count].type = type;
			ide->devices[count].channel = &ide->channels[i];
			ide->devices[count].drive = j;
			ide->devices[count].signature = *signature;
			ide->devices[count].capabilities = *capabilities;
			ide->devices[count].cmd_sets = *cmdsets;
			
			// Size
			if (ide->devices[count].cmd_sets & (1 << 26))
				// Device uses 48-Bit Addressing:
				ide->devices[count].sectors = *((uint32_t*)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
			else
				// Device uses CHS or 28-bit Addressing:
				ide->devices[count].sectors = *((uint32_t*)(ide_buf + ATA_IDENT_MAX_LBA));
			
			//Model name			
			for (int k = 0; k < 40; k += 2)
			{
				ide->devices[count].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
				ide->devices[count].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
			}
			ide->devices[count].model[40] = '\0';
			//trim trailing spaces
			for (int z = 39; z >= 0; z--)
				if (ide->devices[count].model[z] == ' ')
					ide->devices[count].model[z] = '\0';
			
			printf("Chan %d Drive %d Sectors %d type %d cap 0x%x model \'%s\'\n",
				ide->devices[count].channel->index,
				ide->devices[count].drive,
				ide->devices[count].sectors,
				ide->devices[count].type,
				ide->devices[count].capabilities,
				ide->devices[count].model);
			
			_detect_partitions(&ide->devices[count]);

			count++;			
		}
	}

	if (!_ide_controllers[0])
		_ide_controllers[0] = ide;
	else if (!_ide_controllers[1])
		_ide_controllers[1] = ide;
	else
		KPANIC("Too many IDE controllers");
}

uint8_t ide_write_sectors(ide_device_t* ide, uint8_t numsects, uint32_t lba, void* buff)
{
	if (ide->present == 0)
	{
		return 0; //unknown drive
	}

	if (ide->type != IDE_ATA)
	{
		printf("ide_write_sectors Not ATA %d\n", ide->type);
		return 0;
	}

	if ((lba + numsects) > ide->sectors)
	{
		return 0;
	}
	return _ide_ata_access(ide, ATA_WRITE, lba, numsects, buff);
}

uint8_t ide_read_sectors(ide_device_t* ide, uint8_t numsects, uint32_t lba, void* buff)
{
	if (ide->present == 0)
	{
		printf("ide_read_sectors No Drive\n");
		//No drive
		return 0;
	}

	if (ide->type != IDE_ATA)
	{
		printf("ide_read_sectors Not ATA %d drive\n", ide->type);
		return 0;
	}

	if ((lba + numsects) > ide->sectors)
	{
		// Seeking to invalid position.
		printf("ide_read_sectors Invalid pos\n");
		return 0;
	}
	return _ide_ata_access(ide, ATA_READ, lba, numsects, buff);
}

#include <kernel/utils.h>

ide_device_t* ide_get_device(uint8_t controller, uint8_t drive)
{
	bochs_dbg();
	if (controller <= 1 && _ide_controllers[controller] && drive <= 3)
		return &_ide_controllers[controller]->devices[drive];
	return NULL;
}

