#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include "../drivers/console.h"
#include "../drivers/memory.h"
#include "../drivers/keyboard/keyboard.h"
#include <drivers/timer/timer.h>
#include "memory/paging.h"

void kmain()
{
	con_init();
	con_write("Hello World\n");
	gdt_init();
	idt_init();
	fault_init();
	timer_init(1);
	initialise_paging();
	
	kb_init();
	mem_init();
	
	for (;;);
}
