#include "x86/interrupts.h"
#include "x86/gdt.h"

#include "fault.h"

#include <drivers/console.h>
#include "../drivers/memory.h"
#include <drivers/keyboard/keyboard.h>
#include <drivers/timer/timer.h>
#include "memory/paging.h"
#include "memory/kheap.h"

#include <drivers/ata/ata.h>

#include "tasks/task.h"
#include "syscall.h"
#include "utils.h"

typedef struct proc
{
	uint8_t* code;
	uint32_t code_len;

	uint32_t esp, ebp, eip;
	page_directory_t* pages;
	heap_t* heap;

} proc_t;

void run_prog(uint8_t* data, uint32_t len)
{
	page_directory_t* pages = clone_directory(kernel_directory);

	switch_page_directory(pages);

	uint32_t start = 0x00200000;
	uint32_t end = start + 0x1000;
	uint32_t add = start;
	while (add < end)
	{
		alloc_frame(get_page(add, 1, pages), 0, 1);
		memset(add, 0, 0x1000);
		add += 0x1000;
	}
	uint32_t entry = start;
	memcpy((uint8_t*)start, data, len);
	con_printf("copied\n");

	start = 0x00400000;
	end = start + 0x100000;
	add = start;
	while (add < end)
	{
		alloc_frame(get_page(add, 1, pages), 0, 1);
		memset(add, 0, 0x1000);
		add += 0x1000;
	}
	heap_t* h = create_heap(pages, start, end, end, 0, 0);

	uint32_t stack = alloc(0x1000, 1, h) + 0x1000;

	con_printf("Allocated stack at %x\n", stack);
	con_write("switching\n");

	char* tt = (char*)stack;


	//*tt = 'a';
	
	bochs_dbg();
	
	switch_to_user_mode();
	
	//*tt = 'a';
		
	asm volatile ("		\
		mov %0, %%ebx;	\	
		mov %1, %%eax;	\		
		mov %2, %%ebp;	\
		mov %2, %%esp;	\
		jmp %%eax		"
		: : "r"(h), "r" (entry), "r"(stack));
	/*
	
	asm volatile ("mov %0, %%ebp" :: "r"(stack));
	asm volatile ("mov %0, %%esp" :: "r"(stack));

	asm volatile ("mov %0, %%ebx" :: "r"(h));
	asm volatile ("mov %0, %%eax" :: "r"(entry));
	asm volatile ("jmp %eax");
	*/
	/*asm volatile("         \
      cli;                 \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%ebp;       \
      mov %3, %%cr3;       \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx           "
		: : "r"(entry), "r"(stack), "r"(stack), "r"(pages->physicalAddr));
		*/
}

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

	//set_kernel_stack(esp);
	run_prog(buff, 512);

	//ide_initialize(0x1F0, 0x3F4, 0x170, 0x374, 0x000);

	/*page_directory_t* pages = clone_directory(kernel_directory);
	
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

	
	uint32_t mem = alloc(0x1000, 0, h);
	*/
	//con_write("switching\n");

	//switch_to_user_mode();
	
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
