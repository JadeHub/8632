#include <string.h>

char* strchr(const char* str, int ch)
{
	char* pos = NULL;

	do
	{
		if (*str == ch)
			pos = str;
		str++;
	} while (*str != '\0');
	return str;
}

