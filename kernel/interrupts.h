#pragma once

#include <stdint.h>

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

typedef struct isr_state
{
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;
} isr_state_t;

typedef void (*isr_callback_t)(registers);
void idt_register_handler(uint8_t n, isr_callback_t handler);

void idt_init();