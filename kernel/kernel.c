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

#include <drivers/ata/ata.h>
#include <drivers/serial/serial_io.h>

#include <kernel/elf32/elf32.h>
#include <kernel/tasks/proc.h>
#include <kernel/tasks/sched.h>
#include <kernel/debug.h>
#include "syscall.h"
#include "utils.h"
#include "multiboot.h"

#include <kernel/sync/spin_lock.h>

extern void switch_to_user_mode();

uint8_t buff[10000];
uint32_t buf_len;
char exe_name[128];

void kmain(multiboot_data_t* mb_data, uint32_t esp)
{
	con_init();
	con_printf("Hello World %d %x\n", mb_data->mod_count, *((uint8_t*)mb_data->modules->start));
	mb_init(mb_data);
	buf_len = mb_data->modules->end - mb_data->modules->start;
	memcpy(buff, (void*)mb_data->modules->start, buf_len);
	strcpy(exe_name, mb_data->modules->name);
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
	con_write("Task\n");
	kb_init();
	syscall_init();
	ata_init();

	uint8_t atabuff [512];
	ata_read(atabuff, 0, 1);


	con_printf("Hello World %08x %x\n", buff, *((uint8_t*)buff));
	con_printf("Loading elf %x %s\n", buff[0], exe_name);
	elf_image_t* elf = elf_load_raw_image(exe_name, buff, buf_len);
	if (elf)
	{
		con_printf("Loaded elf %s %08x\n", exe_name, elf->entry);
	}

	

	proc_new_proc(elf->exec_section.data, elf->exec_section.sz);
	//proc_new_proc(atabuff, 512);
	
	dbg_mon_init();

	
	//bochs_dbg();
	switch_to_user_mode();
	
	for (;;);
}
