#include "interrupts.h"
//#include "descriptor_tables.h"

#include "../drivers/console.h"
#include "../drivers/memory.h"

void kmain()
{
	con_init();
	idt_init();
	asm volatile ("sti");	// enable interrupt

	//mem_init();
	con_write("Hello World");


	//print_string("Hello testing", 0, 20);
}
