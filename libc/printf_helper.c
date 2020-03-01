#include <sys/printf_helper.h>

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static uint32_t _atoi(const char** str)
{
	uint32_t i = 0U;
	while (isdigit(**str))
	{
		i = i * 10U + (uint32_t)(*((*str)++) - '0');
	}
	return i;
}

static inline int _emit(const char c, flush_fn_t flush, char* buff, int* pos, size_t buff_sz)
{
	buff[*pos] = c;
	(*pos)++;
	if (*pos == buff_sz)
	{
		flush(buff, pos, buff_sz);
	}
	return 1;
}

static int _emit_str(flush_fn_t flush, char* buff, int* buff_pos, size_t buff_sz,
	const char* str, uint32_t len, uint32_t width, uint8_t pad, bool r_justify)
{
	width = width ? width : len;
	int count = 0;
	//prepad
	if (r_justify)
	{
		while (len++ < width)
			count += _emit(pad, flush, buff, buff_pos, buff_sz);
	}
	const char* c = str;
	while (*c && (c - str) < width)
	{
		count += _emit(*c, flush, buff, buff_pos, buff_sz);
		c++;
	}
	//postpad
	if (!r_justify)
	{
		while (len++ < width)
			count += _emit(pad, flush, buff, buff_pos, buff_sz);
	}
	return count;
}

static int _int_out(flush_fn_t flush, char* buff, int* buff_pos, size_t buff_sz,
	uint32_t v, int base, const char* digits, uint32_t width, uint8_t pad, bool r_justify)
{
	char str[64];
	char* p = str;

	uint32_t counter = v;
	do
	{
		p++;
		counter = counter / base;

	} while (counter);
	uint32_t len = p - str;
	*p = '\0';
	do
	{
		*--p = digits[v % base];
		v = v / base;

	} while (v);
	return _emit_str(flush, buff, buff_pos, buff_sz, str, len, width, pad, r_justify);
}

int printf_helper(char* buff, size_t buff_sz, flush_fn_t flush, const char* format, va_list args)
{
	int buff_pos = 0;

	const char* digits = "0123456789ABCDEF";
	uint8_t r_justify;
	uint8_t pad;
	uint32_t width;

	int32_t intv;
	uint8_t chv;
	uint8_t* strv;
	int count = 0;

	//%[flags][width]type
	while (*format != '\0')
	{
		if (*format != '%')
		{
			count += _emit(*format, flush, buff, &buff_pos, buff_sz);
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
			count += _emit(chv, flush, buff, &buff_pos, buff_sz);
			format++;
			break;
		case 'd':
			intv = va_arg(args, int32_t);
			if (intv < 0)
			{
				count += _emit('-', flush, buff, &buff_pos, buff_sz);
				intv *= -1;
			}
			count += _int_out(flush, buff, &buff_pos, buff_sz, intv, 10, digits, width, pad, r_justify);
			format++;
			break;
		case 'u':
			count += _int_out(flush, buff, &buff_pos, buff_sz, va_arg(args, uint32_t), 10, digits, width, pad, r_justify);
			format++;
			break;
		case 'x':
			count += _int_out(flush, buff, &buff_pos, buff_sz, va_arg(args, uint32_t), 16, digits, width, pad, r_justify);
			format++;
			break;
		case 's':
			strv = va_arg(args, char*);
			count += _emit_str(flush, buff, &buff_pos, buff_sz, strv, strlen(strv), width, pad, r_justify);
			format++;
			break;
		case '%':
			count += _emit('%', flush, buff, &buff_pos, buff_sz);
			format++;
			break;
		default:
			count += _emit(*format, flush, buff, &buff_pos, buff_sz);
			format++;
			break;
		}
	}
	if (buff_pos)
		flush(buff, &buff_pos, buff_sz);
	return count;
}
