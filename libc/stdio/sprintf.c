#include <stdio.h>

#include <stdint.h>

/*int snprintf(char* buff, size_t sz, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int count = vsnprintf(buff, sz, format, args);
	va_end(args);
	return count;
}

int sprintf(char* buff, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int count = vsprintf(buff, format, args);
	va_end(args);
	return count;
}

int vsnprintf(char* buff, size_t sz, const char* format, va_list args)
{
	return 0;
}

int vsprintf(char* buff, const char* format, va_list args)
{
	return vsnprintf(buff, SIZE_MAX, format, args);
}*/
