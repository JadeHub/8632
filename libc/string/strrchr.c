#include <string.h>

char* strrchr(const char* str, int ch)
{
	size_t sz = strlen(str);

	if (sz == 0)
		return NULL;

	const char* tmp = str + sz - 1;
	while(tmp != str)
	{
		if (*tmp == ch)
			return (char*)tmp;
		tmp--;
	}

	return NULL;
}

