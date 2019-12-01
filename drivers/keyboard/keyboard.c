//#if 0
#include "keyboard.h"
#include "kb_scancode_tables.h"

#include <kernel/x86/interrupts.h>
#include <drivers/console.h>
#include <drivers/ioports.h>

#include <stdbool.h>

//State
typedef struct state 
{
    bool wait : 1;
    bool lshift : 1;
    bool rshift : 1;
    bool lalt : 1;
    bool ralt : 1;
    bool caps : 1;
} state_t;

state_t kb_state;

static bool is_state_modifier(uint8_t sc)
{
    return sc == 0xE0 ||
        sc == 0x2A ||
        sc == 0xAA ||
        sc == 0x36 ||
        sc == 0xB6 ||
        sc == 0x38 ||
        sc == 0xB8 ||
        sc == 0x38 ||
        sc == 0xB8 ||
        sc == 0x3A ||
        sc == 0xBA; 
}

static bool update_state(uint8_t sc)
{
    if(sc == 0xE0)
        kb_state.wait = true;

    if(!kb_state.wait && sc == 0x2A)
        kb_state.lshift = true;
    if(!kb_state.wait && sc == 0xAA)
        kb_state.lshift = false; 
    
    if(!kb_state.wait && sc == 0x36)
        kb_state.rshift = true;
    if(!kb_state.wait && sc == 0xB6)
        kb_state.rshift = false;

    if(!kb_state.wait && sc == 0x38)
        kb_state.lalt = true;
    if(!kb_state.wait && sc == 0xB8)
        kb_state.lalt = false; 
    
    if(kb_state.wait && sc == 0x38)
        kb_state.ralt = true;
    if(kb_state.wait && sc == 0xB8)
        kb_state.ralt = false;

    if(!kb_state.wait && sc == 0x3A)
        kb_state.caps = true;
    if(!kb_state.wait && sc == 0xBA)
        kb_state.caps = false;
    
    return is_state_modifier(sc);
}

static uint8_t translate(uint8_t sc)
{
    if(kb_state.wait)
        return kb_sc_ascii_2byte[sc];
    if(kb_state.lshift || kb_state.rshift || kb_state.caps)
        return kb_sc_ascii_upper[sc];
    return kb_sc_ascii_lower[sc];    
}

static bool is_special_code(uint8_t sc)
{
    return sc == 0x00 ||
        //sc == 0xAA ||
        sc == 0xEE ||
        sc == 0xFA ||
        sc == 0xFC ||
        sc == 0xFD ||
        sc == 0xFE ||
        sc == 0xFF;
}

static void kb_isr(isr_state_t state)
{
    uint8_t scan_code = inb(0x60);
    if(is_special_code(scan_code))
        return; //todo: 0x00 or 0xFF are error
        
    if(update_state(scan_code))
        return;
 
    if(scan_code & 0x80)
        return; //todo: key release
    
    uint8_t ascii = translate(scan_code);

    if(ascii)
        con_putc(ascii);
    kb_state.wait = false;
}

void kb_init()
{
    idt_register_handler(IRQ1, &kb_isr);
    kb_scancode_tables_init();
}
//#endif