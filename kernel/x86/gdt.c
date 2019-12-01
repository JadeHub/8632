#include "gdt.h"
#include <kernel/utils.h>

#include<drivers/console.h>

#include <stdint.h>

struct gdt_entry_struct
{
	uint16_t limit_low;	// the lower 16bit of the limit
	uint16_t base_low;	// the lower 16bit of the base
	uint8_t  base_middle;	// next 8bits of the base
	uint8_t  access;		// determine which ring segment can be used
	uint8_t  granularity;
	uint8_t  base_high;	// last 8 bits of base
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct
{
	uint16_t limit;		// size of table - 1
	uint32_t base;		// address of first gdt_entry_t struct
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

struct tss_entry_struct
{
   uint32_t prev_tss;
   uint32_t esp0; //kernel stack pointer
   uint32_t ss0;  //kernel stage selector
   uint32_t esp1;
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;
   uint32_t cs;
   uint32_t ss;
   uint32_t ds;
   uint32_t fs;
   uint32_t gs;
   uint32_t ldt;
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;

extern void gdt_flush(uint32_t);
extern void tss_flush();

gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;
tss_entry_t tss_entry;

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0x0F;		// segment len

	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access	= access;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0)
{
   uint32_t base = (uint32_t) &tss_entry;
   uint32_t limit = base + sizeof(tss_entry);

   gdt_set_gate(num, base, limit, 0xE9, 0x00);
   memset(&tss_entry, 0, sizeof(tss_entry));
   tss_entry.ss0  = ss0;
   tss_entry.esp0 = esp0;
   //Kernel code and data segments with RPL of 3
   tss_entry.cs   = 0x0b;
   tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void set_kernel_stack(uint32_t stack)
{
    tss_entry.esp0 = stack;
}

void gdt_init()
{
   gdt_ptr.base = (uint32_t)&gdt_entries;
   gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
	
	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	// code seg
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);	// data seg
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);	// user code seg
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);	// user data seg
   write_tss(5, 0x10, 0x0);

   gdt_flush((uint32_t) &gdt_ptr);
   tss_flush();
}