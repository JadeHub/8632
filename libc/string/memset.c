#include <string.h>

#include <stdint.h>

void* memset(void* dest, int ch, size_t count)
{
    uint8_t* p = (uint8_t*)dest;
    for (; count != 0; count--)
        *p++ = ch;
    return dest;
}