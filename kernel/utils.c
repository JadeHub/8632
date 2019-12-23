#include "utils.h"

void* memset(void* address, uint8_t val, uint32_t len)
{
    uint8_t* p = (uint8_t*)address;
    for(; len != 0; len--)
        *p++ = val;
    return address;
}

void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

uint32_t strlen(const char* buff)
{
	uint32_t res = 0;
	while (*buff++ != '\0')
		res++;
	return res;
}

int strcmp(const char* lhs, const char* rhs)
{
	while (*lhs != 0 && *rhs != 0)
	{
		if (*lhs != *rhs)
			return *lhs - *rhs;
		lhs++;
		rhs++;
	}
	return 0;
}

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