#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include <drivers/console.h>
#include "../drivers/memory.h"
#include <drivers/keyboard/keyboard.h>
#include <drivers/timer/timer.h>
#include "memory/paging.h"
#include "memory/kheap.h"

#include "tasks/task.h"
#include "syscall.h"
#include "utils.h"

//uint32_t t;

//uint8_t prog[64];
extern uint32_t end;

void kmain(uint32_t esp)
{
	int k;
	uint32_t* endp = (uint32_t*)end;
	for (k = 0; k < 64; k++)
	{
	//	prog[k] = endp[k];
	}

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

	page_directory_t* pages = clone_directory(kernel_directory);
	
	switch_page_directory(pages);
	
	uint32_t start = 0x00200000;
	uint32_t end = start + 0x100000;
	uint32_t add = start;
	while (add < end)
	{
		alloc_frame(get_page(add, 1, pages), 0, 1);
		memset(add, 0, 0x1000);
		add += 0x1000;
	}
	heap_t* h = create_heap(pages, start, end, end, 0, 0);

	
	alloc(0x1000, 0, h);

	con_write("switching\n");

	switch_to_user_mode();
	
	//uint32_t b = (uint32_t)prog;
	
//	asm volatile ("mov %0, %%ebx" :: "r"(h));
//	asm volatile ("mov %0, %%eax" :: "r"(b));
//	asm volatile ("jmp %eax");
//	asm volatile("mov $1024, %ecx");
	//asm volatile ("mov %0, %%ebx" :: "r"(h));
	//asm volatile("mov $1, %eax");
	//asm volatile("int $100");

	//asm volatile("mov %%eax, %0" : "=r"(b));

	//asm volatile ("mov %0, %%ebx" :: "r"(b));
	//asm volatile("mov $2, %eax");
	//asm volatile("int $100");

//	alloc(1024, 0, h);

	//alloc()
	//uint32_t* add = 0x1000;
	//uint32_t val = *add; 

	//asm volatile("int $100");

//	mem_init();

	//uint32_t* add = 0xA0000000;
	//uint32_t v = *add;

	for (;;);
}
