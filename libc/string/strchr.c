#include <string.h>

char* strchr(const char* str, int ch)
{
	do
	{
		if (*str == ch)
			return (char*)str;
		str++;
	} while (*str != '\0');
	return NULL;
}

