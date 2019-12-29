#include <string.h>

char* strchr(const char* str, int ch)
{
	while (str && *str != '\0')
	{
		if (*str == ch)
			return (char*)str;
		str++;
	}
	return NULL;
}

