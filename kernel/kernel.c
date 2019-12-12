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

#include "tasks/task.h"
#include "syscall.h"
#include "utils.h"

extern void switch_to_user_mode();

void kmain(uint32_t esp)
{
	con_init();
	con_write("Hello World\n");
	gdt_init();
	idt_init();
	fault_init();
	timer_init(1);
	page_directory_t* kpages = paging_init();	
	
	//bochs_dbg();
	task_init(kpages, esp);
	//bochs_dbg();
	kb_init();
	syscall_init();

	ata_init();

	uint8_t buff [512];

	ata_read(buff, 0, 1);

	con_printf("Read Prog %x %x %x %x\n", buff[0], buff[1], buff[2], buff[3]);

	disable_interrupts();

	for (uint32_t o = 0; o < 10; o++)
	{
		task_new_proc(buff, 512);
	//	task_new_proc(buff, 512);
	}

//	bochs_dbg();
	switch_to_user_mode();
	
	for (;;);
}
