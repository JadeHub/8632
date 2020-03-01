#include "console.h"
#include "display.h"
#include <kernel/utils.h>

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
        if (cursor_x) cursor_x--;
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
