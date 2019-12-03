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
	for (i = 28; i >= 0; i -= 4) 
	{
		int digit = (n >> i) & 0xF;
		if (digit < 10)
			con_putc('0' + digit);
		else
			con_putc('a' + digit - 10);		
	}
}

static void int_out(uint32_t v, int base, const char* digits)
{
	char buff[64];
	char* p = buff;

	uint32_t counter = v;
	do
	{
		p++;
		counter = counter / base;

	}while(counter);

	*p = '\0';
	do
	{
		*--p = digits[v % base];
		v = v / base;

	}while(v);
	con_write(buff);
}

void con_printf(const char* format, ...)
{
	va_list args;

	va_start(args, format);

	const char* digits = "0123456789ABCDEF";

	for (int i = 0; format[i]; i++)
	{
		if (format[i] != '%')
		{
			con_putc(format[i]);
			continue;
		}
		
		i++;
		int32_t intv;

		switch (format[i])
		{
			case 'd':
				intv = va_arg(args, int32_t);
				if (intv < 0)
				{
					con_putc('-');
					intv *= -1;
				}
				int_out(intv, 10, digits);
				break;
			case 'u':
				int_out(va_arg(args, uint32_t), 10, digits);
				break;
			case 'x':
				con_write("0x");
				int_out(va_arg(args, uint32_t), 16, digits);
				break;
			case 's':
				con_write(va_arg(args, char*));
				break;
		}
	}
	va_end(args);
}