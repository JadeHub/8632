#include <string.h>

size_t strlen(const char* buff)
{
	size_t res = 0;
	while (buff && *buff++ != '\0')
		res++;
	return res;
}
