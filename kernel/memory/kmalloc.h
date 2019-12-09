#pragma once

#include <stdint.h>

//alligned on 4k boundry
uint32_t kmalloc_a(uint32_t sz);
//alligned and returning phys address
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

uint32_t kmalloc(uint32_t sz);
void kfree(void *p);
