#include <stdlib.h>

#include <stdint.h>
#include <ctype.h>

int atoi(const char* str)
{
	while (isspace(*str)) str++;
	int i = 0;
	while (*str >= '0' && *str <= '9')
	{
		i = i * 10U + (uint32_t)(*(str++) - '0');
	}
	return i;
}