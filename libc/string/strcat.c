#include <string.h>

char* strcat(char* dest, const char* src)
{
	while (*dest != '\0')
		dest++;
	return strcpy(dest, src);
}