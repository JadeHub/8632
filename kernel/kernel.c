#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include <drivers/console.h>
#include <drivers/memory.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/timer/timer.h>
#include <drivers/ata/ata.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/kheap.h>
#include <kernel/dbg_monitor/dbg_monitor.h>
#include <kernel/ramfs/ramfs.h>

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

#include <kernel/sync/spin_lock.h>

extern void switch_to_user_mode();

void list_test()
{
	typedef struct Foo
	{
		int bar;
		list_head_t list;
	}Foo_t;

	//Declare a list
	
	list_head_t my_list;
	INIT_LIST_HEAD(&my_list);
	//my_list = LIST_HEAD_INIT(my_list);
//		LIST_HEAD(my_list);


	con_printf("Empty = %d\n", list_empty(&my_list));

	Foo_t f1;
	f1.bar = 1;
	list_add(&f1.list, &my_list);


	con_printf("Empty = %d\n", list_empty(&my_list));

	Foo_t f2;
	f2.bar = 2;
	list_add(&f2.list, &my_list);

	Foo_t f3;
	f3.bar = 3;
	list_add(&f3.list, &my_list);

	list_head_t* item;
	list_for_each_rev(item, &my_list)
	{
		Foo_t* foo = list_entry(item, Foo_t, list);
	//	con_printf("Foo %d\n", foo->bar);
		if(foo->bar == 3)
		{
			list_delete(&foo->list);
			break;
		}
	}

	Foo_t* f;
	list_for_each_entry(f, &my_list, list)
	{
		con_printf("Foo %d\n", f->bar);
	}
}

#include <kernel/fs/fs.h>
#include <kernel/fs/dir.h>

void dump_dir(fs_node_t* d);

bool write_fs(fs_node_t* p, fs_node_t* c)
{
	if ((c->flags & FS_DIR))
	{
		dump_dir(c);
	}
	else
	{
		con_printf("DUMP file %s in %s \n", c->name, p->name);
	}
	return true;
}

void dump_dir(fs_node_t* d)
{
	con_printf("DUMP dumping %s\n", d->name);
	fs_read_dir(d, &write_fs);
}

uint8_t buff[10000];
uint32_t buf_len = 10000;
char exe_name[128];

uint8_t ram_disk_buff[0x4000];
uint32_t ram_disk_len = 0x4000;

void kmain(multiboot_data_t* mb_data, uint32_t esp)
{
	con_init();
	list_test();
	con_printf("Hello World %d %x\n", mb_data->mod_count, *((uint8_t*)mb_data->modules->start));
	mb_init(mb_data);

	//buf_len = mb_copy_mod("/boot/user_space", buff, buf_len);
	//if(!buf_len)
	//	KPANIC("Module /boot/user_space not found\n");
	

	ram_disk_len = mb_copy_mod("/boot/ramdisk", ram_disk_buff, ram_disk_len);
	if (!ram_disk_len)
		KPANIC("Module /boot/ramdisk not found\n");
	con_printf("Ram Disk %08x bytes\n", ram_disk_len);
	elf_image_t* k_image = mb_get_kernel_elf();
	dbg_init(k_image);
	//bochs_dbg();
	gdt_init();
	con_write("gdt\n");
	idt_init();
	con_write("idt\n");
	fault_init();
	serial_init();
	timer_init(1);
	con_write("timer\n");
	page_directory_t* kpages = paging_init();
	con_write("paging\n");
	proc_init(kpages, esp);
	sched_init(proc_kernel_proc());
	con_write("sched\n");
	fs_init();
	con_write("fs\n");
	ramfs_init(ram_disk_buff, ram_disk_len);
	dump_dir(ramfs_root());	
	con_write("initrd\n");
	kb_init();
	syscall_init();
	con_write("ata\n");
	ata_init();

	uint8_t atabuff [512];
	ata_read(atabuff, 0, 1);

	fs_node_t* bin = fs_find_child(ramfs_root(), "bin");
	if (bin)
	{
		fs_node_t* f = fs_find_child(bin, "user_space");
		if(f)
		{
			strcpy(exe_name, "/bin/user_space");
			fs_read(f, buff, 0, buf_len);
			con_printf("Loading elf %x %d %s\n", buff[0], buf_len, exe_name);
			proc_new_elf_proc(exe_name, buff, buf_len);
		}
	}
	

	//proc_new_proc(elf->exec_section.data, elf->exec_section.sz);
	//proc_new_proc(atabuff, 512);
	
	dbg_mon_init();

	
	//bochs_dbg();
	switch_to_user_mode();
	
	for (;;);
}
