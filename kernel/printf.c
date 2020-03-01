#include <kernel/utils.h>

#include <ctype.h>
#include <stdio.h>

static void _emit_str(void (*emit)(char), const char* str, uint32_t len, uint32_t width, uint8_t pad, bool r_justify)
{
	width = width ? width : len;
	//prepad
	if (r_justify)
	{
		while (len++ < width)
			(*emit)(pad);
	}
	const char* c = str;
	while (*c && (c-str) < width)
	{
		(emit)(*c);
		c++;
	}
	//postpad
	if (!r_justify)
	{
		while (len++ < width)
			(*emit)(pad);
	}
}

static void _int_out(uint32_t v, int base, const char* digits, uint32_t width, uint8_t pad, bool r_justify, void (*emit)(char))
{
	char buff[64];
	char* p = buff;

	uint32_t counter = v;
	do
	{
		p++;
		counter = counter / base;

	} while (counter);
	uint32_t len = p - buff;
	*p = '\0';
	do
	{
		*--p = digits[v % base];
		v = v / base;

	} while (v);
	_emit_str(emit, buff, len, width, pad, r_justify);
}

static uint32_t _atoi(const char** str)
{
	uint32_t i = 0U;
	while (isdigit(**str))
	{
		i = i * 10U + (uint32_t)(*((*str)++) - '0');
	}
	return i;
}

void printf_helper(void (*emit)(char), const char* format, va_list args)
{
	const char* digits = "0123456789ABCDEF";
	uint8_t r_justify;
	uint8_t pad;
	uint32_t width;

	int32_t intv;
	uint8_t chv;
	uint8_t* strv;

	//%[flags][width]type
	while(*format != '\0')
	{
		if (*format != '%')
		{
			(*emit)(*format);
			format++;
			continue;
		}
		format++;

		//flags
		r_justify = true;
		pad = ' ';
		if (*format == '-')
		{
			r_justify = false;
			format++;
		}
		else if (*format == '0')
		{
			pad = '0';
			format++;
		}

		//field width
		width = 0;
		if (isdigit(*format))
		{
			width = _atoi(&format);
		}

		switch (*format)
		{
		case 'c':
			chv = va_arg(args, int);
			(*emit)(chv);
			format++;
			break;
		case 'd':
			intv = va_arg(args, int32_t);
			if (intv < 0)
			{
				(*emit)('-');
				intv *= -1;
			}
			_int_out(intv, 10, digits, width, pad, r_justify, emit);
			format++;
			break;
		case 'u':
			_int_out(va_arg(args, uint32_t), 10, digits, width, pad, r_justify, emit);
			format++;
			break;
		case 'x':
			//0x
			(*emit)('0');
			(*emit)('x');
			_int_out(va_arg(args, uint32_t), 16, digits, width, pad, r_justify, emit);
			format++;
			break;
		case 's':
			strv = va_arg(args, char*);
			_emit_str(emit, strv, strlen(strv), width, pad, r_justify);
			format++;
			break;
		case '%':
			(*emit)('%');
			format++;
			break;
		default:
			(*emit)(*format);
			format++;
			break;
		}
	}
}

char* sprintf_buff = 0;

void sprintf_emit(char c)
{
	*sprintf_buff = c;
	sprintf_buff++;
	*sprintf_buff = 0;
}

void vsprintf(char* buff, const char* format, va_list args)
{
	sprintf_buff = buff;
	printf_helper(&sprintf_emit, format, args);
}

void sprintf(char* buff, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);
}
