#include "console.h"
#include "display.h"
#include <kernel/utils.h>
#include <drivers/ioports.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/device_driver.h>
#include <kernel/fault.h>
#include <kernel/io/io.h>

#include <stdio.h>

typedef struct console
{
    dev_device_t device;
    uint8_t cursor_x;
    uint8_t cursor_y;

    uint32_t kbd_fd;
}console_t;

static dev_driver_t _driver;
static console_t _console;

static void _open_con(dev_device_t* d, uint32_t flags)
{
    printf("opening console\n");
 //   _console.cursor_x = _console.cursor_y = 0;
   // dsp_clear_screen();
   // dsp_set_cursor(_console.cursor_x, _console.cursor_y);

    
    _console.kbd_fd = open("/dev/keyboard", 0);
    //printf("opened kbd %d\n", _console.kbd_fd);
    ASSERT(_console.kbd_fd != 0xffffffff);
}

static void _close_con(dev_device_t* d)
{
    close(_console.kbd_fd);
}

static size_t _read_con(dev_device_t* d, uint8_t* buff, size_t off, size_t sz)
{
    kb_event_t kbe;
    for (size_t i = 0; i < sz; i++)
    {
       read(_console.kbd_fd, (uint8_t*)&kbe, sizeof(kb_event_t));

       if (kbe.pressed)
       {
           buff[i] = kbe.code;
           con_putc(buff[i]);
           if (buff[i] == '\n')
           {
               buff[i] = '\0';
               break;
           }
       }
    }
    
    return sz;
}

static size_t _write_con(dev_device_t* d, uint8_t* buff, size_t off, size_t sz)
{
    for (size_t i = off; i < (off + sz); i++)
    {
        con_putc(buff[i]);
    }
    return sz;
}

void con_init()
{
    dsp_clear_screen();
    dsp_set_cursor(_console.cursor_x, _console.cursor_y);
}

void con_dev_init()
{
    memset(&_driver, 0, sizeof(dev_driver_t));
    memset(&_console, 0, sizeof(console_t));
    kname_set("console_driver", &_driver.name);
    kname_set("console", &_console.device.name);
    _driver.read = &_read_con;
    _driver.write = &_write_con;
    _driver.open = &_open_con;
    _driver.close = &_close_con;
    _console.device.driver = &_driver;

    dev_install_driver(&_driver);
    dev_register_device(&_console.device);
}

void con_putc(char c)
{
    if (c == 0x08) //backspace
    {        
        if (_console.cursor_x > 0)
        {            
            _console.cursor_x--;
            dsp_print_char(' ', _console.cursor_x, _console.cursor_y);
        }
    }
    else if(c == 0x09) //tab
    {
        _console.cursor_x += 4;
    }
    else if(c == '\r')
    {
        _console.cursor_x = 0;
    }
    else if(c == '\n')
    {
        _console.cursor_x = 0;
        _console.cursor_y++;
    }
    else
    {
        dsp_print_char(c, _console.cursor_x, _console.cursor_y);
        _console.cursor_x++;
    }    

    if(_console.cursor_x >= 80)
    {
        _console.cursor_x = 0;
        _console.cursor_y++;
    }

    if (_console.cursor_y == 25)
    {
        dsp_scroll(1);
        _console.cursor_y = 24;
    }

    dsp_set_cursor(_console.cursor_x, _console.cursor_y);
}

void con_write_buff(const char* buff, size_t sz)
{
    for (size_t i = 0; i < sz; i++)
        con_putc(buff[i]);
}

void con_enable_cursor()
{
    uint8_t cursor_start = 12;
    uint8_t cursor_end = 15;
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void con_disable_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void con_set_cursor(uint8_t col, uint8_t row)
{
    dsp_set_cursor(col, row);
}