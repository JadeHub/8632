#include <string.h>

#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t count)
{
    const uint8_t* sp = (const uint8_t*)src;
    uint8_t* dp = (uint8_t*)dest;
    for (; count != 0; count--) *dp++ = *sp++;
    return dest;
}