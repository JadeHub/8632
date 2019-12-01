#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include "../drivers/console.h"
#include "../drivers/memory.h"
#include "../drivers/keyboard/keyboard.h"
#include <drivers/timer/timer.h>
#include "memory/paging.h"

#include "tasks/task.h"
#include "syscall.h"

void kmain(uint32_t esp)
{
	con_init();
	con_write("Hello World\n");
	gdt_init();
	idt_init();
	fault_init();
	timer_init(1);
	initialise_paging();
	
	kb_init();

	task_init(esp);

	syscall_init();
	switch_to_user_mode();

	asm volatile("int $100");

//	mem_init();

//	uint32_t* add = 0xA0000000;
//	uint32_t v = *add;

	for (;;);
}
