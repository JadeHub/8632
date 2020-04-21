#include <string.h>

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
    int i = 0;
    while (*s1 && (*s1 == *s2) && i < count)
    {
        s1++;
        s2++;
        i++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}