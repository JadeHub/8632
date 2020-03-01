#include <stdio.h>

#include "syscall.h"

#include <ctype.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define PRINTF_BUFF_SZ 128

typedef void (*flush_fn_t)(const char*, uint32_t);

static uint32_t _atoi(const char** str)
{
	uint32_t i = 0U;
	while (isdigit(**str))
	{
		i = i * 10U + (uint32_t)(*((*str)++) - '0');
	}
	return i;
}

static inline bool _add_to_buff(const char c, char* buff, int* pos)
{
	buff[*pos] = c;
	(*pos)++;
	return (*pos == PRINTF_BUFF_SZ);
}

static inline int _emit(const char c, flush_fn_t flush, char* buff, int* pos)
{
	if (_add_to_buff(c, buff, pos))
	{
		flush(buff, PRINTF_BUFF_SZ);
		*pos = 0;
	}
	return 1;
}

static int _emit_str(flush_fn_t flush, char* buff, int* buff_pos,
	const char* str, uint32_t len, uint32_t width, uint8_t pad, bool r_justify)
{
	width = width ? width : len;
	int count = 0;
	//prepad
	if (r_justify)
	{
		while (len++ < width)
			count += _emit(pad, flush, buff, buff_pos);
	}
	const char* c = str;
	while (*c && (c - str) < width)
	{
		count += _emit(*c, flush, buff, buff_pos);
		c++;
	}
	//postpad
	if (!r_justify)
	{
		while (len++ < width)
			count += _emit(pad, flush, buff, buff_pos);
	}
	return count;
}

static int _int_out(flush_fn_t flush, char* buff, int* buff_pos,
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
	return _emit_str(flush, buff, buff_pos, str, len, width, pad, r_justify);
}

static int _printf_helper(flush_fn_t flush, const char* format, va_list args)
{
	char buff[PRINTF_BUFF_SZ];
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
			count += _emit(*format, flush, buff, &buff_pos);
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
			count += _emit(chv, flush, buff, &buff_pos);
			format++;
			break;
		case 'd':
			intv = va_arg(args, int32_t);
			if (intv < 0)
			{
				count += _emit('-', flush, buff, &buff_pos);
				intv *= -1;
			}
			count += _int_out(flush, buff, &buff_pos, intv, 10, digits, width, pad, r_justify);
			format++;
			break;
		case 'u':
			count += _int_out(flush, buff, &buff_pos, va_arg(args, uint32_t), 10, digits, width, pad, r_justify);
			format++;
			break;
		case 'x':
			count += _int_out(flush, buff, &buff_pos, va_arg(args, uint32_t), 16, digits, width, pad, r_justify);
			format++;
			break;
		case 's':
			strv = va_arg(args, char*);
			count += _emit_str(flush, buff, &buff_pos, strv, strlen(strv), width, pad, r_justify);
			format++;
			break;
		case '%':
			count += _emit('%', flush, buff, &buff_pos);
			format++;
			break;
		default:
			count += _emit(*format, flush, buff, &buff_pos);
			format++;
			break;
		}
	}
	if (buff_pos)
		flush(buff, buff_pos);
	return count;
}

static void _print_str_flush(const char* buff, uint32_t len)
{
	SYSCALL2(SYSCALL_PRINT_STR, buff, len);
}

int testj() { return 5; }

int printf(const char* format, ...)
{
	//SYSCALL2(SYSCALL_PRINT_STR, format, strlen(format));
	va_list args;
	va_start(args, format);
	int count = _printf_helper(&_print_str_flush, format, args);
	//SYSCALL1(SYSCALL_PRINT, format);

	va_end(args);
	return 0;
}