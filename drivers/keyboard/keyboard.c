#include "keyboard.h"
#include "kb_scancode_tables.h"
#include <kernel/fault.h>
#include <kernel/utils.h>
#include <kernel/debug.h>
#include <kernel/x86/interrupts.h>
#include <kernel/types/kname.h>
#include <drivers/ioports.h>
#include <drivers/device_driver.h>

#include <stdio.h>
#include <stdbool.h>

#define BUFF_SZ 128

static kb_event_t _events[BUFF_SZ];
static uint8_t _events_head = 0;
static uint8_t _events_tail = 0;
static bool _events_full = false;

static bool _events_empty()
{
    return !_events_full && _events_head == _events_tail;
}

static void _put_event(kb_event_t* e)
{
    _events[_events_head] = *e;
    if (_events_full)
        _events_tail = (_events_tail + 1) % BUFF_SZ;
    _events_head = (_events_head + 1) % BUFF_SZ;
    _events_full = _events_head == _events_tail;
}

static bool _get_event(kb_event_t* e)
{
    kb_event_t* result = NULL;

    if (_events_empty())
        return false;
    
    *e = _events[_events_tail];
    _events_full = false;
    _events_tail = (_events_tail + 1) % BUFF_SZ;
    return true;
}

//we only support a sinlge keyboard so the device and driver are one
typedef struct kb_device
{
    dev_driver_t driver;
    dev_device_t device;    
    kb_state_t state;
    bool wait_2nd;  //Are we waiting for a second byte from the keyboard

    kb_event_handler observer;

    //thread_t* waiters;
}kb_device_t;

static kb_device_t _kb;

static size_t _read_keyboard(dev_device_t* d, uint8_t* buff, size_t off, size_t sz)
{
    ASSERT(sz % sizeof(kb_event_t) == 0);

    size_t i = 0;
    for (;i < sz; i += sizeof(kb_event_t))
    {
        while (_events_empty())
            dev_block_until_read(&_kb.device);

        if (!_get_event((kb_event_t*)buff + i))
            break;
    }
    return i;
}

static bool _is_state_modifier(uint8_t sc)
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
        sc == 0xBA ||
        sc == 0x1D ||
        sc == 0x9D;
}

static bool _update_state(uint8_t sc)
{
    if(sc == 0xE0)
        _kb.wait_2nd = true;

    if(!_kb.wait_2nd && sc == 0x2A)
        _kb.state.lshift = true;
    if(!_kb.wait_2nd && sc == 0xAA)
        _kb.state.lshift = false; 
    
    if(!_kb.wait_2nd && sc == 0x36)
        _kb.state.rshift = true;
    if(!_kb.wait_2nd && sc == 0xB6)
        _kb.state.rshift = false;

    if(!_kb.wait_2nd && sc == 0x38)
        _kb.state.lalt = true;
    if(!_kb.wait_2nd && sc == 0xB8)
        _kb.state.lalt = false; 
    
    if(_kb.wait_2nd && sc == 0x38)
        _kb.state.ralt = true;
    if(_kb.wait_2nd && sc == 0xB8)
        _kb.state.ralt = false;

    if(!_kb.wait_2nd && sc == 0x3A)
        _kb.state.caps = true;
    if(!_kb.wait_2nd && sc == 0xBA)
        _kb.state.caps = false;

    if (!_kb.wait_2nd && sc == 0x1D)
        _kb.state.lctrl = true;
    if (!_kb.wait_2nd && sc == 0x9D)
        _kb.state.lctrl = false;

    if (_kb.wait_2nd && sc == 0x1D)
        _kb.state.rctrl = true;
    if (_kb.wait_2nd && sc == 0x9D)
        _kb.state.rctrl = false;
    
    //return false;
    return _is_state_modifier(sc);
}

static uint8_t _translate(uint8_t sc)
{
    if(_kb.wait_2nd)
        return kb_sc_ascii_2byte[sc];
    if(_kb.state.lshift || _kb.state.rshift || _kb.state.caps)
        return kb_sc_ascii_upper[sc];
    return kb_sc_ascii_lower[sc];    
}

static bool _is_special_code(uint8_t sc)
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

static void kb_isr(isr_state_t* state)
{
    uint8_t scan_code = inb(0x60);

  //  printf("K 0x%x\n", scan_code);
    if (_is_special_code(scan_code))
        return; //todo: 0x00 or 0xFF are error

    if (scan_code == 0xE0)
    {
        _kb.wait_2nd = true;
        return;
    }
        
    if (_update_state(scan_code))
        return;
 
    bool pressed = !(scan_code & 0x80);
    scan_code &= 0x7F; //upper bit indicates key release

    uint8_t ascii = _translate(scan_code);
    if (ascii)
    {
        kb_event_t e;
        e.code = ascii;
        e.state = _kb.state;
        e.pressed = pressed;
        _put_event(&e);

        if (_kb.observer)
            _kb.observer(&e);

        dev_unblock_readers(&_kb.device);
    }
    _kb.wait_2nd = false;
}

void kb_init()
{
    memset(&_kb, 0, sizeof(kb_device_t));
    idt_register_handler(IRQ1, &kb_isr);
    kb_scancode_tables_init();

    kname_set("kb_driver", &_kb.driver.name);
    kname_set("keyboard", &_kb.device.name);

    _kb.driver.read = &_read_keyboard;
    _kb.device.driver = &_kb.driver;

    dev_install_driver(&_kb.driver);
    dev_register_device(&_kb.device);
}

void kb_register_event_handler(kb_event_handler cb)
{
    _kb.observer = cb;
}

void kb_remove_event_handler()
{
    _kb.observer = NULL;
}
