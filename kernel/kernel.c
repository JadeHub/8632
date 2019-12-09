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
	uint32_t id;

} proc_t;

proc_t proc1, proc2;

uint8_t do_sw;

void switch_proc(proc_t* p)
{

}

/*
void on_timer(isr_state_t* regs)
{
	if(!do_sw) return;

	proc_t* p;
	if (do_sw == 2)
	{
		switch_proc(&proc1);
	}
	else
	{
		switch_proc(&proc2);	
	}





	if (do_sw == 2)
		do_sw = 1;
	else
		do_sw = 2;

	switch_page_directory(proc1.pages);

	uint32_t h = proc1.id;
	uint32_t ip = proc1.eip;
	uint32_t st = proc1.esp;

	asm volatile ("		\
		cli;			\
		mov %0, %%ebx;	\	
		mov %1, %%eax;	\
		mov %2, %%ebp;	\
		mov %2, %%esp;	\
		sti;			\
		jmp %%eax		"
		: : "r"(h), "r" (ip), "r"(st));

	do_sw = 0;
}*/

void run_prog(uint8_t* data, uint32_t len, proc_t* p, uint32_t id)
{
	page_directory_t* pages = clone_directory(kernel_directory);

	switch_page_directory(pages);

	p->code = data;
	p->code_len = len;
	p->pages = pages;
	p->id = id;

	uint32_t start = 0x00200000;
	uint32_t end = start + 0x1000;
	alloc_pages(pages, start, end);
	uint32_t entry = start;
	memcpy((uint8_t*)start, data, len);
	con_printf("copied\n");

	start = 0x00400000;
	end = start + 0x100000;
	alloc_pages(pages, start, end);
	heap_t* h = create_heap(pages, start, end, end, 0, 0);

	uint32_t stack = (uint32_t)alloc(0x1000, 1, h) + 0x1000;

	con_printf("Allocated stack at %x\n", stack);
	con_write("switching\n");

	p->esp = p->ebp = stack;
	p->eip = entry;
	p->heap = h;

	bochs_dbg();
	do_sw = 2;

	switch_page_directory(kernel_directory);

	switch_to_user_mode();
	/*	
	asm volatile ("		\
		mov %0, %%ebx;	\	
		mov %1, %%eax;	\		
		mov %2, %%ebp;	\
		mov %2, %%esp;	\
		jmp %%eax		"
		: : "r"(h), "r" (entry), "r"(stack));*/
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
	do_sw = 0;

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

	task_new_proc(buff, 512);
	task_new_proc(buff, 512);
	//run_prog(buff, 512, &proc1, 1);	

	switch_to_user_mode();


	for (;;);
}
