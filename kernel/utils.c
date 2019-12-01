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

void bochs_dbg()
{
    asm volatile("xchg %bx, %bx");
}