#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include <drivers/console.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/timer/timer.h>
#include <drivers/ata/ata.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/kheap.h>
#include <kernel/dbg_monitor/dbg_monitor.h>
#include <kernel/ramfs/ramfs.h>
#include <kernel/devfs/devfs.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/dir.h>

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

uint8_t buff[20000];
uint32_t buf_len = 20000;
char exe_name[128];

uint8_t ram_disk_buff[0x4000];
uint32_t ram_disk_len = 0x4000;


void kmain(multiboot_data_t* mb_data, uint32_t esp)
{
	con_init();
	printf("Hello World %d 0x%x\n", mb_data->mod_count, *((uint8_t*)mb_data->modules->start));
	mb_init(mb_data);
	ram_disk_len = mb_copy_mod("/boot/ramdisk", ram_disk_buff, ram_disk_len);
	if (!ram_disk_len)
		KPANIC("Module /boot/ramdisk not found\n");
	printf("Ram Disk 0x%08x bytes\n", ram_disk_len);
	elf_image_t* k_image = mb_get_kernel_elf();
	dbg_init(k_image);
	gdt_init();
	printf("gdt\n");
	idt_init();
	printf("idt\n");
	fault_init();

	io_init();

	serial_init();
	timer_init(100, &ktimer_cb);
	printf("timer\n");
	page_directory_t* kpages = paging_init();
	printf("paging\n");
	proc_init(kpages, esp);
	sched_init(proc_kernel_proc());
	printf("sched\n");
	fs_init();
	printf("fs\n");
	ramfs_init(ram_disk_buff, ram_disk_len);
	printf("initrd\n");
	devfs_init();
	kb_init();
	syscall_init();
	printf("ata\n");
	ata_init();
	time_init();

	uint8_t atabuff [512];
	ata_read(atabuff, 0, 1);

	fs_node_t* parent;
	fs_node_t* f = fs_get_abs_path("/initrd/bin/user_space", &parent);
	if(f)
	{
		ASSERT(f->len <= buf_len);
		strcpy(exe_name, "user_space");
		fs_read(f, buff, 0, buf_len);
		printf("Loading elf 0x%x %d %s/%s\n", buff[0], buf_len, parent->name, exe_name);
		proc_new_elf_proc(exe_name, buff, buf_len);
	}

	dbg_mon_init();
	switch_to_user_mode();

	
	for (;;);
}
