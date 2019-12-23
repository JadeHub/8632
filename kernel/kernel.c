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

#include "tasks/task.h"
#include "syscall.h"
#include "utils.h"
#include "multiboot.h"

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
		con_printf("MB Has ELF Symbols %x\n", data->elf_sections.size);
	if (data->flags & MULTIBOOT_INFO_MEM_MAP)
		con_printf("MB Has MMap\n");
	if (data->flags & MULTIBOOT_INFO_DRIVE_INFO)
		con_printf("MB Has Drive info\n");
	if (data->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME)
		con_printf("MB Bootloader %s\n", data->bootloader_name);
}

void kmain(multiboot_data_t* mb_data, uint32_t esp)
{
	con_init();
	con_write("Hello World\n");

	con_printf("PT:%x\n", 0x12345);

	dump_mb(mb_data);
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
	con_write("Task\n");
	kb_init();
	syscall_init();
	ata_init();

	uint8_t buff [512];
	ata_read(buff, 0, 1);

	con_printf("Read Prog %x %x %x %x\n", buff[0], buff[1], buff[2], buff[3]);

	task_new_proc(buff, 512);
	task_new_proc(buff, 512);
	//con_printf("writing\n");
	//KLOG(LL_ERR, "KERNEL", "Initialisation %x", 0);

	dbg_mon_init();

	//bochs_dbg();
	switch_to_user_mode();
	
	for (;;);
}
