#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include <drivers/console/console.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/timer/timer.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/ide.h>
//#include <drivers/ata/ata.h>
#include <drivers/display.h>
#include <kernel/memory/phys_mem.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/kheap.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/ramfs/ramfs.h>
#include <kernel/devfs/devfs.h>
#include <kernel/vfs/vfs.h>
#include <kernel/vfs/dir.h>

#include <drivers/ata/ata.h>
#include <drivers/serial/serial_io.h>

#include <kernel/elf32/elf32.h>
#include <kernel/tasks/proc.h>
#include <kernel/tasks/sched.h>
#include <kernel/debug.h>
#include "syscall.h"
#include "utils.h"
#include "multiboot.h"
#include "types/list.h"
#include "types/hash_tbl.h"
#include "types/kname.h"
#include <kernel/sync/spin_lock.h>
#include <kernel/io/io.h>
#include <kernel/time.h>
#include <kernel/ktimer.h>

#include <stdio.h>

extern void switch_to_user_mode();

#define RAM_DISK_LEN 0x20000

uint8_t ram_disk_buff[RAM_DISK_LEN];
uint32_t ram_disk_len = RAM_DISK_LEN;

extern uint32_t end;

void kmain(multiboot_data_t* mb_data, uint32_t esp)
{
	con_init();
	printf("Hello World %d 0x%x\n", mb_data->mod_count, *((uint8_t*)mb_data->modules->start));
	printf("Buffer 0x%08x End 0x%08x\n", ram_disk_buff, end);
	memset(ram_disk_buff, 0, RAM_DISK_LEN);
	mb_init(mb_data);
	ram_disk_len = mb_copy_mod("/boot/ramdisk", ram_disk_buff, ram_disk_len);
	if (!ram_disk_len)
		KPANIC("Module /boot/ramdisk not found\n");
	printf("Ram Disk 0x%08x bytes\n", ram_disk_len);
	
	//bochs_dbg();

	elf_image_t* k_image = mb_get_kernel_elf();
	dbg_init(k_image);
	gdt_init();
	idt_init();
	fault_init();
	
	serial_init();
	timer_init(1000, &ktimer_cb);
	phys_mem_init();
	
	
	
	
	page_directory_t* kpages = paging_init();

	io_init();

	proc_init(kpages, esp, k_image);
	sched_init(proc_kernel_proc());
	fs_init();
	ramfs_init(ram_disk_buff, ram_disk_len);
	devfs_init();
	kb_init();
	con_dev_init();
	syscall_init();
	pci_init();
	//ata_init();
	time_init();
	/*
	uint8_t dbuff[1024];
	memset(dbuff, 1, 1024);
	dbuff[0] = 0xA;
	dbuff[1] = 0xB;
	dbuff[2] = 0xC;
	dbuff[3] = 0xD;
	dbuff[4] = 0xE;

	dbuff[0 + 512] = 0xD;
	dbuff[1 + 512] = 0xE;
	dbuff[2 + 512] = 0xA;
	dbuff[3 + 512] = 0xD;
	dbuff[4 + 512] = 0xF;

//	ide_write(2, 0, dbuff);

	uint8_t dbuff2[1024];
	ide_read(2, 0, dbuff2);
	
	printf("IDE Read sec 1 0x%x 0x%x 0x%x 0x%x 0x%x\n", dbuff2[0], dbuff2[1], dbuff2[2], dbuff2[3], dbuff2[4]);
	printf("IDE Read sec 2 0x%x 0x%x 0x%x 0x%x 0x%x\n", dbuff2[0+512], dbuff2[1 + 512], dbuff2[2 + 512], dbuff2[3 + 512], dbuff2[4 + 512]);
	*/
	const char* args[2];
	args[0] = "shell";
	args[1] = NULL;
	uint32_t fds[3];
	fds[0] = fds[1] = fds[2] = INVALID_FD;

	proc_start_user_proc("/initrd/bin/shell", args, fds);
	dsp_enable_cursor();
	switch_to_user_mode();

	for (;;);
}
