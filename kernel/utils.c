#include "utils.h"

void* memset(void* address, uint8_t val, uint32_t len)
{
    uint8_t* p = (uint8_t*)address;
    for(; len != 0; len--)
        *p++ = val;
    return address;
}