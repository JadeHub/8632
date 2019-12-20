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

#include <drivers/ata/ata.h>
#include <drivers/serial/serial_io.h>

#include "tasks/task.h"
#include "syscall.h"
#include "utils.h"

extern void switch_to_user_mode();

void kmain(uint32_t esp)
{
	con_init();
	con_write("Hello World2\n");
	gdt_init();
	con_write("gdt\n");
	idt_init();
	con_write("idt\n");
	fault_init();
	//serial_init();
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
	con_printf("writing\n");
	KLOG(LL_ERR, "KERNEL", "Initialisation %x", 0);

	//bochs_dbg();
	switch_to_user_mode();
	
	for (;;);
}
