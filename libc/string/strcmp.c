#include <string.h>

#include <stdio.h>

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t count)
{
    while (*s1 && (*s1 == *s2) && count > 0)
    {
        s1++;
        s2++;
        count--;
    }
    if (!count)
        return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}