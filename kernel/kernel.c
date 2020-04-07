#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include <drivers/console/console.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/timer/timer.h>
#include <drivers/ata/ata.h>
#include <drivers/display.h>
#include <kernel/memory/phys_mem.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/kheap.h>
#include <kernel/memory/kmalloc.h>
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

uint8_t ram_disk_buff[0x10000];
uint32_t ram_disk_len = 0x20000;

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
	idt_init();
	fault_init();
	io_init();
	serial_init();
	timer_init(1000, &ktimer_cb);
	phys_mem_init();
	page_directory_t* kpages = paging_init();
	proc_init(kpages, esp, k_image);
	sched_init(proc_kernel_proc());
	fs_init();
	ramfs_init(ram_disk_buff, ram_disk_len);
	devfs_init();
	kb_init();
	con_dev_init();
	syscall_init();
	ata_init();
	time_init();
	
	fs_node_t* parent;
	const char* path = "/initrd/bin/shell";
	fs_node_t* f = fs_get_abs_path(path, &parent);
	if(f)
	{
		uint8_t* exe_buff = (uint8_t*)kmalloc(f->len);
		ASSERT(exe_buff);
		fs_read(f, exe_buff, 0, f->len);
		proc_new_elf_proc("shell", exe_buff, f->len);
	}
	
	dsp_enable_cursor();
	switch_to_user_mode();

	for (;;);
}
