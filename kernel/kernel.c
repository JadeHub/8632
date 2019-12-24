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

void dump_mb(multiboot_data_t* data)
{
	con_printf("MB Flags %x\n", data->flags);
	if (data->flags & MULTIBOOT_INFO_MEMORY)
		con_printf("MB Mem low %x high %x\n", data->mem_low, data->mem_high);
	if (data->flags & MULTIBOOT_INFO_BOOTDEV)
		con_printf("MB Boot device %x\n", data->boot_device);
	if (data->flags & MULTIBOOT_INFO_CMDLINE)
		con_printf("MB Command line %s\n", data->cmdline);
	if (data->flags & MULTIBOOT_INFO_ELF_SHDR)
	{
		con_printf("ELF Symbols count %d\n", data->elf_sections.count);
		con_printf("ELF Symbols size %d\n", data->elf_sections.size);
		con_printf("ELF Symbols address %08x\n", data->elf_sections.address);
		con_printf("ELF Symbols shndx %x\n", data->elf_sections.shndx);
	}
	if (data->flags & MULTIBOOT_INFO_MEM_MAP)
		con_printf("MB Has MMap %x\n", data->mmap);
	if (data->flags & MULTIBOOT_INFO_DRIVE_INFO)
		con_printf("MB Has Drive info\n");
	if (data->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME)
		con_printf("MB Bootloader %s\n", data->bootloader_name);

	mem_map_data_t* m = data->mmap ;
	
	while (m < data->mmap + data->mmap_size)
	{
		//con_printf("Memory: sz %x address %08x end %08x len %x type %d\n", m->sz, m->address_low, (m->address_low + m->size_low), m->size_low, m->type);
		//m = (mem_map_data_t*)((uint32_t)m + m->sz + sizeof(m->sz));

	}

	bochs_dbg();
	
}

extern uint32_t end;
uint32_t t = (uint32_t)&end;

void kmain(multiboot_data_t* mb_data, uint32_t esp)
{
	con_init();
	con_write("Hello World\n");

	mb_init(mb_data);
	elf32_image_t* k_image = mb_get_kernel_elf32();
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
	task_init(kpages, esp);
	sched_init(task_kernel_proc());
	con_write("Task\n");
	kb_init();
	syscall_init();
	ata_init();

	uint8_t buff [512];
	ata_read(buff, 0, 1);

	con_printf("Read Prog %x %x %x %x\n", buff[0], buff[1], buff[2], buff[3]);

	task_new_proc(buff, 512);
	task_new_proc(buff, 512);
	
	dbg_mon_init();

	
	//bochs_dbg();
	switch_to_user_mode();
	
	for (;;);
}
