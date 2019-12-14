#pragma once

#include <stdint.h>

#define ISR13 13
#define ISR14 14

#define ISR_SYSCALL 100

#define IRQ0 32
#define IRQ1 33
#define IRQ4 36

typedef struct isr_state
{
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;
} isr_state_t;

typedef void (*isr_callback_t)(isr_state_t*);
void idt_register_handler(uint8_t n, isr_callback_t handler);

void idt_init();

void enable_interrupts();
void disable_interrupts();