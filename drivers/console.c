#include "console.h"
#include "display.h"
#include <kernel/utils.h>
#include <drivers/ioports.h>

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

void con_init()
{    
    dsp_clear_screen();
    dsp_set_cursor(cursor_x, cursor_y);
}

void con_putc(char c)
{
    if (c == 0x08) //backspace
    {        
        if (cursor_x)
        {            
            cursor_x--;
            dsp_print_char(' ', cursor_x, cursor_y);
        }
    }
    else if(c == 0x09) //tab
    {
        cursor_x += 4;
    }
    else if(c == '\r')
    {
        cursor_x = 0;
    }
    else if(c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        dsp_print_char(c, cursor_x, cursor_y);
        cursor_x++;
    }    

    if(cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y == 25)
    {
        dsp_scroll(1);
        cursor_y = 24;
    }

    dsp_set_cursor(cursor_x, cursor_y);
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