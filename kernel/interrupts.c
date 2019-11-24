#include "interrupts.h"
#include "utils.h"
#include "../drivers/ioports.h"
#include "../drivers/console.h"

idt_entry_t idt_entries[256];
idt_table_ptr_t idt_ptr;
isr_callback_t isr_handlers[256];

static int MASTER_IRQ_COMMAND = 0x20;
static int MASTER_IRQ_DATA = 0x21;
static int SLAVE_IRQ_COMMAND = 0xA0;
static int SLAVE_IRQ_DATA = 0xA1;

extern void idt_flush(uint32_t);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

static void idt_set_entry(uint8_t num, uint32_t base)
{
    idt_entries[num].base_high = (base >> 16) && 0xFFFF;
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].seg_sel = 0x08; //kernel code selector
    idt_entries[num].flags = 0x8E;
    idt_entries[num].unused = 0;
}

void idt_init()
{
    memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
    memset(&isr_handlers, 0, sizeof(isr_callback_t)*256);

    con_write("test ");
    con_write_hex(0);
    con_write("\n");

    idt_ptr.base = &idt_entries;
    idt_ptr.limit = (sizeof(idt_entry_t)*256)-1;

    idt_set_entry(0, (uint32_t) isr0);
	idt_set_entry(1, (uint32_t) isr1);
	idt_set_entry(2, (uint32_t) isr2);
	idt_set_entry(3, (uint32_t) isr3);
	idt_set_entry(4, (uint32_t) isr4);
	idt_set_entry(5, (uint32_t) isr5);
	idt_set_entry(6, (uint32_t) isr6);
	idt_set_entry(7, (uint32_t) isr7);
	idt_set_entry(8, (uint32_t) isr8);
	idt_set_entry(9, (uint32_t) isr9);
	idt_set_entry(10, (uint32_t) isr10);
	idt_set_entry(11, (uint32_t) isr11);
	idt_set_entry(12, (uint32_t) isr12);
	idt_set_entry(13, (uint32_t) isr13);
	idt_set_entry(14, (uint32_t) isr14);
	idt_set_entry(15, (uint32_t) isr15);
	idt_set_entry(16, (uint32_t) isr16);
	idt_set_entry(17, (uint32_t) isr17);
	idt_set_entry(18, (uint32_t) isr18);
	idt_set_entry(19, (uint32_t) isr19);
	idt_set_entry(20, (uint32_t) isr20);
	idt_set_entry(21, (uint32_t) isr21);
	idt_set_entry(22, (uint32_t) isr22);
	idt_set_entry(23, (uint32_t) isr23);
	idt_set_entry(24, (uint32_t) isr24);
	idt_set_entry(25, (uint32_t) isr25);
	idt_set_entry(26, (uint32_t) isr26);
	idt_set_entry(27, (uint32_t) isr27);
	idt_set_entry(28, (uint32_t) isr28);
	idt_set_entry(29, (uint32_t) isr29);
	idt_set_entry(30, (uint32_t) isr30);
	idt_set_entry(31, (uint32_t) isr31);

	// remap IRQ table
	outb(MASTER_IRQ_COMMAND, 0x11);		// initialize master IRQ
	outb(SLAVE_IRQ_COMMAND, 0x11);		// initialize slave IRQ
	outb(MASTER_IRQ_DATA, 0x20);		// vector offset
	outb(SLAVE_IRQ_DATA, 0x28);		// vector offset
	outb(MASTER_IRQ_DATA, 0x04);		// tell there's slave IRQ at 0x0100
	outb(SLAVE_IRQ_DATA, 0x02);		// tell it's cascade identity
	outb(MASTER_IRQ_DATA, 0x01);		// 8086 mode
	outb(SLAVE_IRQ_DATA, 0x01);		// 8086 mode
	outb(MASTER_IRQ_DATA, 0x0);
	outb(SLAVE_IRQ_DATA, 0x0);

	idt_set_entry(32, (uint32_t) irq0);	//Timer
	idt_set_entry(33, (uint32_t) irq1); //Keyboard
	idt_set_entry(34, (uint32_t) irq2);
	idt_set_entry(35, (uint32_t) irq3); //Serial Port 2
	idt_set_entry(36, (uint32_t) irq4); //Serial Port 1
	idt_set_entry(37, (uint32_t) irq5);
	idt_set_entry(38, (uint32_t) irq6);
	idt_set_entry(39, (uint32_t) irq7); //Real Time Clock
	idt_set_entry(40, (uint32_t) irq8);
	idt_set_entry(41, (uint32_t) irq9);
	idt_set_entry(42, (uint32_t) irq10);
	idt_set_entry(43, (uint32_t) irq11);
	idt_set_entry(44, (uint32_t) irq12);
	idt_set_entry(45, (uint32_t) irq13);
	idt_set_entry(46, (uint32_t) irq14);
	idt_set_entry(47, (uint32_t) irq15);

    idt_flush((uint32_t) &idt_ptr);

  
}


void idt_register_handler(uint8_t n, isr_callback_t handler)
{
    isr_handlers[n] = handler;
}

void isr_handler(isr_state_t regs)
{
    con_write("isr ");
    con_write_hex(regs.int_no);
    con_write("\n");
	if (isr_handlers[regs.int_no] != 0)
    {
		isr_callback_t handler = isr_handlers[regs.int_no];
		handler(regs);
	}
    else 
    {
		con_write("unhandled isr: ");
		con_write_hex(regs.int_no);
	    con_write("\n");
	}
}



void irq_handler(isr_state_t regs)
{
    con_write("irq ");
    con_write_hex(regs.int_no-32);
	con_write("\n");
	if (regs.int_no >= 40) 
	{
		outb(0xA0, 0x20);	// send reset signal to slave
	}

	outb(0x20, 0x20);		// send reset signal to master

	if (isr_handlers[regs.int_no] != 0)
	{
		isr_callback_t handler = isr_handlers[regs.int_no];
		handler(regs);
	}
}