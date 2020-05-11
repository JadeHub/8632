#include <string.h>

int memcmp(const void* lhs, const void* rhs, size_t count)
{
	const char* str1 = (const char*)lhs;
	const char* str2 = (const char*)rhs;
	for (size_t i = 0; i < count; i++)
	{
		if (str1[i] != str2[i])
			return str1[i] - str2[i];
	}
	return 0;
}