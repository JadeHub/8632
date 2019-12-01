#pragma once

#include <stdint.h>

void* memset(void* address, uint8_t val, uint32_t len);
void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len);

void bochs_dbg();