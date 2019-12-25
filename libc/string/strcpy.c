#include <string.h>

char* strcpy(char* dest, const char* source)
{
	char* ret = dest;

	while (*source != '\0')
	{
		*dest++ = *source++;
	}
	*dest = '\0';
	return ret;
}
