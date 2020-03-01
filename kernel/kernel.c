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

#include <stdio.h>

extern void switch_to_user_mode();

void hash_test()
{
	typedef struct Bar
	{
		kname_t name;
		hash_tbl_item_t hash_item;
	}Bar_t;

	hash_tbl_t* ht = hash_tbl_create(256);

	Bar_t b1;
	kname_set("B 1", &b1.name);
	hash_tbl_add(ht, 1, &b1.hash_item);

	Bar_t b2;
	kname_set("B 511", &b2.name);
	hash_tbl_add(ht, 511, &b2.hash_item);

	Bar_t b3;
	kname_set("B 2048", &b3.name);
	hash_tbl_add(ht, 2048, &b3.hash_item);

	hash_tbl_item_t* i = hash_tbl_find(ht, 511);

	if (i)
	{
		
		Bar_t* test = container_of(i, Bar_t, hash_item);
		printf("found %s\n", test->name.str);
	}

	Bar_t* t2 = hash_tbl_lookup(ht, 2048, Bar_t, hash_item);
	if (t2)
		printf("Found %s\n", t2->name.str);
}

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

	printf("Empty = %d\n", list_empty(&my_list));

	Foo_t f1;
	f1.bar = 1;
	list_add(&f1.list, &my_list);


	printf("Empty = %d\n", list_empty(&my_list));

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
	//	printf("Foo %d\n", foo->bar);
		if(foo->bar == 3)
		{
			list_delete(&foo->list);
			break;
		}
	}

	Foo_t* f;
	list_for_each_entry(f, &my_list, list)
	{
		printf("Foo %d\n", f->bar);
	}
}

uint8_t buff[20000];
uint32_t buf_len = 20000;
char exe_name[128];

uint8_t ram_disk_buff[0x4000];
uint32_t ram_disk_len = 0x4000;

void kmain(multiboot_data_t* mb_data, uint32_t esp)
{
	con_init();
	//list_test();
	//hash_test();
	printf("Hello World %d 0x%x\n", mb_data->mod_count, *((uint8_t*)mb_data->modules->start));
	//bochs_dbg();
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
	timer_init(1);
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

	uint8_t atabuff [512];
	ata_read(atabuff, 0, 1);

	fs_node_t* parent;
	fs_node_t* f = fs_get_abs_path("/initrd/bin/user_space", &parent);
	if(f)
	{
		strcpy(exe_name, "user_space");
		fs_read(f, buff, 0, buf_len);
		printf("Loading elf 0x%x %d %s/%s\n", buff[0], buf_len, parent->name, exe_name);
		proc_new_elf_proc(exe_name, buff, buf_len);
	}
	
	dbg_mon_init();
	switch_to_user_mode();
	
	for (;;);
}
