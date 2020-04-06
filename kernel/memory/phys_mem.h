#pragma once

typedef struct page page_t;

void phys_mem_init();

void alloc_frame(page_t* page, int is_kernel, int is_writeable);
void free_frame(page_t* page);