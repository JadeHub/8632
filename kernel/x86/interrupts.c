#include "interrupts.h"
#include <kernel/utils.h>
#include <drivers/ioports.h>
#include <drivers/console.h>

#include "isr.h"

struct idt_entry {

    uint16_t base_low;
    uint16_t seg_sel;
    uint8_t unused;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

typedef struct idt_entry idt_entry_t;

struct idt_table_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct idt_table_ptr idt_table_ptr_t;

static idt_entry_t idt_entries[256];
static idt_table_ptr_t idt_ptr;
static isr_callback_t isr_handlers[256];

static int MASTER_IRQ_COMMAND = 0x20;
static int MASTER_IRQ_DATA = 0x21;
static int SLAVE_IRQ_COMMAND = 0xA0;
static int SLAVE_IRQ_DATA = 0xA1;

static void _idt_set_entry(uint8_t num, uint32_t base)
{
    idt_entries[num].base_high = (base >> 16) && 0xFFFF;
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].seg_sel = 0x08; //kernel code selector
    idt_entries[num].flags = 0x8E | 0x60;
    idt_entries[num].unused = 0;
}

static inline bool _is_irq(uint32_t isr)
{
	return isr >= 32 && isr <= 47;
}

void idt_init()
{
    memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
    memset(&isr_handlers, 0, sizeof(isr_callback_t)*256);

    idt_ptr.base = (uint32_t)&idt_entries;
    idt_ptr.limit = (sizeof(idt_entry_t)*256)-1;

    _idt_set_entry(0, (uint32_t) isr0); //Divide error
	_idt_set_entry(1, (uint32_t) isr1); //Debug
	_idt_set_entry(2, (uint32_t) isr2); //NMI Interrupt
	_idt_set_entry(3, (uint32_t) isr3); //Breakpoint
	_idt_set_entry(4, (uint32_t) isr4); //Overflow
	_idt_set_entry(5, (uint32_t) isr5); //Bound range exceeded
	_idt_set_entry(6, (uint32_t) isr6); //Invalid Opcode
	_idt_set_entry(7, (uint32_t) isr7); //Divice Not Available
	_idt_set_entry(8, (uint32_t) isr8); //Double Fault
	_idt_set_entry(9, (uint32_t) isr9); //Coprocessor error
	_idt_set_entry(10, (uint32_t) isr10); //Invalid TSS
	_idt_set_entry(11, (uint32_t) isr11); //Segment not present
	_idt_set_entry(12, (uint32_t) isr12); //Stack Segment Fault
	_idt_set_entry(13, (uint32_t) isr13); //GP Fault
	_idt_set_entry(14, (uint32_t) isr14); //Page Fault
	_idt_set_entry(15, (uint32_t) isr15); //Reserved
	_idt_set_entry(16, (uint32_t) isr16); //Floating point error
	_idt_set_entry(17, (uint32_t) isr17); //Alignment check
	_idt_set_entry(18, (uint32_t) isr18); //Machine check
	_idt_set_entry(19, (uint32_t) isr19); //SIMD Floating point exception
	_idt_set_entry(20, (uint32_t) isr20); //Virtualisation exception
	_idt_set_entry(21, (uint32_t) isr21); //Control protection exception
	_idt_set_entry(22, (uint32_t) isr22);
	_idt_set_entry(23, (uint32_t) isr23);
	_idt_set_entry(24, (uint32_t) isr24);
	_idt_set_entry(25, (uint32_t) isr25);
	_idt_set_entry(26, (uint32_t) isr26);
	_idt_set_entry(27, (uint32_t) isr27);
	_idt_set_entry(28, (uint32_t) isr28);
	_idt_set_entry(29, (uint32_t) isr29);
	_idt_set_entry(30, (uint32_t) isr30);
	_idt_set_entry(31, (uint32_t) isr31);
	_idt_set_entry(100, (uint32_t)isr100);

	//Setup PIC
	outb(MASTER_IRQ_COMMAND, 0x11);
	outb(SLAVE_IRQ_COMMAND, 0x11);
	//Offsets
	outb(MASTER_IRQ_DATA, 0x20);	//32 ~ 39
	outb(SLAVE_IRQ_DATA, 0x28);		//40 ~ 47
	outb(MASTER_IRQ_DATA, 0x04);
	outb(SLAVE_IRQ_DATA, 0x02);
	outb(MASTER_IRQ_DATA, 0x01);
	outb(SLAVE_IRQ_DATA, 0x01);
	outb(MASTER_IRQ_DATA, 0x0);
	outb(SLAVE_IRQ_DATA, 0x0);

	//IRQs
	_idt_set_entry(32, (uint32_t) isr32); //Timer
	_idt_set_entry(33, (uint32_t) isr33); //Keyboard
	_idt_set_entry(34, (uint32_t) isr34); //Cascade
	_idt_set_entry(35, (uint32_t) isr35); //Serial Port 2
	_idt_set_entry(36, (uint32_t) isr36); //Serial Port 1
	_idt_set_entry(37, (uint32_t) isr37); //LPT2
	_idt_set_entry(38, (uint32_t) isr38); //FDD
	_idt_set_entry(39, (uint32_t) isr39); //LPT1 / Spurious
	_idt_set_entry(40, (uint32_t) isr40); //Real Time Clock
	_idt_set_entry(41, (uint32_t) isr41);
	_idt_set_entry(42, (uint32_t) isr42);
	_idt_set_entry(43, (uint32_t) isr43);
	_idt_set_entry(44, (uint32_t) isr44); //PS2 Mouse
	_idt_set_entry(45, (uint32_t) isr45); //FPU
	_idt_set_entry(46, (uint32_t) isr46); //Primary ATA
	_idt_set_entry(47, (uint32_t) isr47); //Secondary ATA / Spurious

    idt_flush((uint32_t) &idt_ptr);
}

void idt_register_handler(uint8_t n, isr_callback_t handler)
{
    isr_handlers[n] = handler;
}

void isr_handler(isr_state_t* regs)
{
	if (_is_irq(regs->int_no))
	{
		//handle spurious irqs https://wiki.osdev.org/8259_PIC#Spurious_IRQs
		if (regs->int_no >= 40)
			outb(0xA0, 0x20);	// send reset signal to slave
		outb(0x20, 0x20);		// send reset signal to master
	}
	if (isr_handlers[regs->int_no] != 0)
    {
		isr_callback_t handler = isr_handlers[regs->int_no];
		handler(regs);
	}
}