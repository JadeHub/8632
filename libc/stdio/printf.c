#include <stdio.h>

#include <sys/printf_helper.h>
#include <sys/syscall.h>

int vprintf(const char* format, va_list args)
{
	char buff[1024];
	size_t len = printf_helper(buff, 1204, format, args);
	sys_print_str(buff, len);
	return (int)len;
}

int printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int count = vprintf(format, args);
	va_end(args);
	return count;
}