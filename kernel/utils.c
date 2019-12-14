#include "utils.h"

void* memset(void* address, uint8_t val, uint32_t len)
{
    uint8_t* p = (uint8_t*)address;
    for(; len != 0; len--)
        *p++ = val;
    return address;
}

void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

static void emit_str(void (*emit)(char), const char* str)
{
	const char* c = str;

	while (*c)
	{
		(emit)(*c);
		c++;
	}
}

static void int_out(uint32_t v, int base, const char* digits, void (*emit)(char))
{
	char buff[64];
	char* p = buff;

	uint32_t counter = v;
	do
	{
		p++;
		counter = counter / base;

	} while (counter);

	*p = '\0';
	do
	{
		*--p = digits[v % base];
		v = v / base;

	} while (v);
	emit_str(emit, buff);
}

void printf_helper(void (*emit)(char), const char* format, va_list args)
{
	const char* digits = "0123456789ABCDEF";

	for (int i = 0; format[i]; i++)
	{
		if (format[i] != '%')
		{
			(*emit)(format[i]);
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
				(*emit)('-');
				intv *= -1;
			}
			int_out(intv, 10, digits, emit);
			break;
		case 'u':
			int_out(va_arg(args, uint32_t), 10, digits, emit);
			break;
		case 'x':
			emit_str(emit, "0x");
			int_out(va_arg(args, uint32_t), 16, digits, emit);
			break;
		case 's':
			emit_str(emit, va_arg(args, char*));
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