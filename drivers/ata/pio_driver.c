#if 0
#include "pio_driver.h"
#include <drivers/ioports.h>
#include <drivers/console.h>

unsigned char ide_buf[2048] = { 0 };
unsigned static char ide_irq_invoked = 0;

void sleep(uint32_t t)
{

}

void ide_write(unsigned char channel, unsigned char reg, unsigned char data)
{
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	if (reg < 0x08)
		outb(data, channels[channel].base + reg - 0x00);
	else if (reg < 0x0C)
		outb(data, channels[channel].base + reg - 0x06);
	else if (reg < 0x0E)
		outb(data, channels[channel].ctrl + reg - 0x0A);
	else if (reg < 0x16)
		outb(data, channels[channel].bmide + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

unsigned char ide_read(unsigned char channel, unsigned char reg)
{
	unsigned char result;
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	if (reg < 0x08)
		result = inb(channels[channel].base + reg - 0x00);
	else if (reg < 0x0C)
		result = inb(channels[channel].base + reg - 0x06);
	else if (reg < 0x0E)
		result = inb(channels[channel].ctrl + reg - 0x0A);
	else if (reg < 0x16)
		result = inb(channels[channel].bmide + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
	return result;
}

void ide_read_buffer(unsigned char channel, unsigned char reg, void* buffer, unsigned int quads)
{
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	asm("pushw %es; movw %ds, %ax; movw %ax, %es");
	if (reg < 0x08)
		insl(channels[channel].base + reg - 0x00, buffer, quads);
	else if (reg < 0x0C)
		insl(channels[channel].base + reg - 0x06, buffer, quads);
	else if (reg < 0x0E)
		insl(channels[channel].ctrl + reg - 0x0A, buffer, quads);
	else if (reg < 0x16)
		insl(channels[channel].bmide + reg - 0x0E, buffer, quads);
	asm("popw %es;");
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

unsigned char ide_print_error(unsigned int drive, unsigned char err) {

	if (err == 0) return err;

	con_write(" IDE:");
	if (err == 1) { con_write("- Device Fault\n     "); err = 19; }
	else if (err == 2) {
		unsigned char st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);
		if (st & ATA_ER_AMNF) { con_write("- No Address Mark Found\n     ");   err = 7; }
		if (st & ATA_ER_TK0NF) { con_write("- No Media or Media Error\n     ");   err = 3; }
		if (st & ATA_ER_ABRT) { con_write("- Command Aborted\n     ");      err = 20; }
		if (st & ATA_ER_MCR) { con_write("- No Media or Media Error\n     ");   err = 3; }
		if (st & ATA_ER_IDNF) { con_write("- ID mark not Found\n     ");      err = 21; }
		if (st & ATA_ER_MC) { con_write("- No Media or Media Error\n     ");   err = 3; }
		if (st & ATA_ER_UNC) { con_write("- Uncorrectable Data Error\n     ");   err = 22; }
		if (st & ATA_ER_BBK) { con_write("- Bad Sectors\n     ");       err = 13; }
	}
	else  if (err == 3) { con_write("- Reads Nothing\n     "); err = 23; }
	else  if (err == 4) { con_write("- Write Protected\n     "); err = 8; }
	con_printf("- [%s %s] %s\n",
		(const char *[]) {
		"Primary", "Secondary"
	}[ide_devices[drive].channel],
			(const char *[]) {
			"Master", "Slave"
		}[ide_devices[drive].drive],
				ide_devices[drive].model);

	return err;
}

unsigned char ide_polling(unsigned char channel, unsigned int advanced_check) {

	// (I) Delay 400 nanosecond for BSY to be set:
	// -------------------------------------------------
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
	ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.

										  // (II) Wait for BSY to be cleared:
										  // -------------------------------------------------
	while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait for BSY to be zero.

	if (advanced_check) {

		unsigned char state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.

																 // (III) Check For Errors:
																 // -------------------------------------------------
		if (state & ATA_SR_ERR) return 2; // Error.

										  // (IV) Check If Device fault:
										  // -------------------------------------------------
		if (state & ATA_SR_DF) return 1; // Device Fault.

										 // (V) Check DRQ:
										 // -------------------------------------------------
										 // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
		if (!(state & ATA_SR_DRQ)) return 3; // DRQ should be set

	}

	return 0; // No Error.

}

void ide_initialize(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3,
	unsigned int BAR4)
{

	int i, j, k, count = 0;

	// 1- Detect I/O Ports which interface IDE Controller:
	channels[ATA_PRIMARY].base = (BAR0 &= 0xFFFFFFFC) + 0x1F0 * (!BAR0);
	channels[ATA_PRIMARY].ctrl = (BAR1 &= 0xFFFFFFFC) + 0x3F4 * (!BAR1);
	channels[ATA_SECONDARY].base = (BAR2 &= 0xFFFFFFFC) + 0x170 * (!BAR2);
	channels[ATA_SECONDARY].ctrl = (BAR3 &= 0xFFFFFFFC) + 0x374 * (!BAR3);
	channels[ATA_PRIMARY].bmide = (BAR4 &= 0xFFFFFFFC) + 0; // Bus Master IDE
	channels[ATA_SECONDARY].bmide = (BAR4 &= 0xFFFFFFFC) + 8; // Bus Master IDE
															  // 2- Disable IRQs:
	ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
	ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
	
	// 3- Detect ATA-ATAPI Devices:
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{

			unsigned char err = 0, type = IDE_ATA, status;
			ide_devices[count].reserved = 0; // Assuming that no drive here.

			bochs_dbg();
											 // (I) Select Drive:
			ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
			sleep(1); // Wait 1ms for drive select to work.


			bochs_dbg();
					  // (II) Send ATA Identify Command:
			ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			sleep(1); // This function should be implemented in your OS. which waits for 1 ms. it is based on System Timer Device Driver.

					  // (III) Polling:
			if (!(ide_read(i, ATA_REG_STATUS))) continue; // If Status = 0, No Device.

			while (1) {
				status = ide_read(i, ATA_REG_STATUS);
				if ((status & ATA_SR_ERR)) { err = 1; break; } // If Err, Device is not ATA.
				if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
			}

			// (IV) Probe for ATAPI Devices:

			if (err) {
				unsigned char cl = ide_read(i, ATA_REG_LBA1);
				unsigned char ch = ide_read(i, ATA_REG_LBA2);

				if (cl == 0x14 && ch == 0xEB) type = IDE_ATAPI;
				else if (cl == 0x69 && ch == 0x96) type = IDE_ATAPI;
				else continue; // Unknown Type (And always not be a device).

				ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
				sleep(1);
			}

			// (V) Read Identification Space of the Device:
			ide_read_buffer(i, ATA_REG_DATA, (void*)ide_buf, 128);

			// (VI) Read Device Parameters:
			ide_devices[count].reserved = 1;
			ide_devices[count].type = type;
			ide_devices[count].channel = i;
			ide_devices[count].drive = j;
			ide_devices[count].sign = ((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE))[0];
			ide_devices[count].capabilities = ((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES))[0];
			ide_devices[count].commandsets = ((unsigned int   *)(ide_buf + ATA_IDENT_COMMANDSETS))[0];

			// (VII) Get Size:
			if (ide_devices[count].commandsets & (1 << 26)) {
				// Device uses 48-Bit Addressing:
				ide_devices[count].size = ((unsigned int   *)(ide_buf + ATA_IDENT_MAX_LBA_EXT))[0];
				// Note that Quafios is 32-Bit Operating System, So last 2 Words are ignored.
			}
			else {
				// Device uses CHS or 28-bit Addressing:
				ide_devices[count].size = ((unsigned int   *)(ide_buf + ATA_IDENT_MAX_LBA))[0];
			}

			// (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
			for (k = ATA_IDENT_MODEL; k < (ATA_IDENT_MODEL + 40); k += 2) {
				ide_devices[count].model[k - ATA_IDENT_MODEL] = ide_buf[k + 1];
				ide_devices[count].model[(k + 1) - ATA_IDENT_MODEL] = ide_buf[k];
			}
			ide_devices[count].model[40] = 0; // Terminate String.

			count++;
			break;
		}
		break;
	}

	// 4- Print Summary:
	for (i = 0; i < 4; i++)
		if (ide_devices[i].reserved == 1) 
		{
			con_printf(" Found %s Drive %dGB - %s\n",
				(const char *[]) {
				"ATA", "ATAPI"
			}[ide_devices[i].type],         /* Type */
					ide_devices[i].size / 1024 / 1024 / 2,               /* Size */
					ide_devices[i].model);
		}
}

#endif