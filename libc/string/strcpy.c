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


char* strncpy(char* dest, const char* source, size_t len)
{
	char* ret = dest;
	size_t pos = 0;
	while (*source != '\0' && pos < len)
	{
		*dest++ = *source++;
		pos++;
	}
	while (pos < len)
	{
		*dest++ = '\0';
		pos++;
	}
	return ret;
}
