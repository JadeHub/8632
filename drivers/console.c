#include "console.h"
#include "display.h"

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

void con_write(const char *str)
{
    const char *c = str;
    while(*c)
    {
        con_putc(*c);
        c++;
    }
}

void con_write_hex(uint32_t n)
{
	int i;
	for (i = 28; i >= 0; i -= 4) {
		int digit = (n >> i) & 0xF;
		if (digit < 10) {
			con_putc('0' + digit);
		} else {
			con_putc('a' + digit - 10);
		}
	}
}