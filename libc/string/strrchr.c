#include <string.h>

char* strrchr(const char* str, int ch)
{
	while (str && *str != '\0')
	{
		if (*str == ch)
			return (char*)str;
		str++;
	}
	return NULL;
}

