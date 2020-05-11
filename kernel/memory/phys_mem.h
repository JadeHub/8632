#pragma once

#include <stdint.h>

typedef struct page page_t;

void phys_mem_init();

void alloc_frame(page_t* page, int is_kernel, int is_writeable);
void free_frame(page_t* page);

#define FRAME_SIZE 0x1000

void pmm_init(uint32_t mb);
uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t frame);