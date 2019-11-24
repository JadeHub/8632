#include "utils.h"

void* memset(void* address, uint8_t val, uint32_t len)
{
    uint8_t* p = address;
    for(uint32_t i=0; i<len; i++)
        p[i] = val;
    return address;
}