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

static inline int _emit(const char c, char* buff, size_t* pos, size_t buff_sz)
{
	if ((*pos) < buff_sz)
	{
		buff[*pos] = c;
		(*pos)++;
	}
	return 1;
}

static int _emit_str(char* buff, size_t* buff_pos, size_t buff_sz,
	const char* str, uint32_t len, uint32_t width, uint8_t pad, bool r_justify)
{
	width = width ? width : len;
	int count = 0;
	//prepad
	if (r_justify)
	{
		while (len++ < width)
			count += _emit(pad, buff, buff_pos, buff_sz);
	}
	const char* c = str;
	while (*c && (c - str) < width)
	{
		count += _emit(*c, buff, buff_pos, buff_sz);
		c++;
	}
	//postpad
	if (!r_justify)
	{
		while (len++ < width)
			count += _emit(pad, buff, buff_pos, buff_sz);
	}
	return count;
}

static int _int_out(char* buff, size_t* buff_pos, size_t buff_sz,
	uint64_t v, int base, const char* digits, uint32_t width, uint8_t pad, bool r_justify)
{
	char str[64];
	char* p = str;

	uint64_t counter = v;
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
	return _emit_str(buff, buff_pos, buff_sz, str, len, width, pad, r_justify);
}

int printf_helper(char* buff, size_t buff_sz, const char* format, va_list args)
{
	size_t buff_pos = 0;

	static const char* digits = "0123456789ABCDEF";
	uint8_t r_justify;
	uint8_t pad;
	uint32_t width;

	int32_t intv;
	int64_t longlongv;
	uint8_t chv;
	uint8_t* strv;
	int count = 0;
	bool _long = false;
	bool _long_long = false;

	//%[flags][width]type
	while (*format != '\0')
	{
		if (*format != '%')
		{
			count += _emit(*format, buff, &buff_pos, buff_sz);
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

		if (*format == 'l')
		{
			_long = true;
			format++;
		}
		if (*format == 'l')
		{
			_long_long = true;
			format++;
		}

		switch (*format)
		{
		case 'c':
			chv = va_arg(args, int);
			count += _emit(chv, buff, &buff_pos, buff_sz);
			format++;
			break;
		case 'd':
			if (_long_long)
			{
				longlongv = va_arg(args, int64_t);
				if (longlongv < 0)
				{
					count += _emit('-', buff, &buff_pos, buff_sz);
					longlongv *= -1;
				}
				count += _int_out(buff, &buff_pos, buff_sz, longlongv, 10, digits, width, pad, r_justify);
			}
			else
			{
				intv = va_arg(args, int32_t);
				if (intv < 0)
				{
					count += _emit('-', buff, &buff_pos, buff_sz);
					intv *= -1;
				}
				count += _int_out(buff, &buff_pos, buff_sz, intv, 10, digits, width, pad, r_justify);
			}
			format++;
			break;
		case 'u':
			if (_long_long)
				count += _int_out(buff, &buff_pos, buff_sz, va_arg(args, uint64_t), 10, digits, width, pad, r_justify);
			else
				count += _int_out(buff, &buff_pos, buff_sz, va_arg(args, uint32_t), 10, digits, width, pad, r_justify);
			format++;
			break;
		case 'x':
			if (_long_long)
				count += _int_out(buff, &buff_pos, buff_sz, va_arg(args, uint64_t), 16, digits, width, pad, r_justify);
			else
				count += _int_out(buff, &buff_pos, buff_sz, va_arg(args, uint32_t), 16, digits, width, pad, r_justify);
			format++;
			break;
		case 's':
			strv = va_arg(args, char*);
			count += _emit_str(buff, &buff_pos, buff_sz, strv, strlen(strv), width, pad, r_justify);
			format++;
			break;
		case '%':
			count += _emit('%', buff, &buff_pos, buff_sz);
			format++;
			break;
		default:
			count += _emit(*format, buff, &buff_pos, buff_sz);
			format++;
			break;
		}
	}
	return buff_pos;
}