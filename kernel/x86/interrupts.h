#pragma once

#include <stdint.h>

#define ISR6 6
#define ISR13 13
#define ISR14 14

#define ISR_SYSCALL 100

#define IRQ0 32
#define IRQ1 33
#define IRQ4 36

typedef struct isr_state
{
	uint32_t ds;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t int_no;
	uint32_t err_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t useresp;
	uint32_t ss;
} isr_state_t;

typedef void (*isr_callback_t)(isr_state_t*);
void idt_register_handler(uint8_t n, isr_callback_t handler);

void idt_init();

static inline void enable_interrupts()
{
	asm volatile ("sti");
}

static inline void disable_interrupts()
{
	asm volatile ("cli");
}