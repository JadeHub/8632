#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct page
{
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw         : 1;   // Read-only if clear, readwrite if set
    uint32_t user       : 1;   // Supervisor level only if clear
    uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
    uint32_t dirty      : 1;   // Has the page been written to since last refresh?
    uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
    uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table
{
    page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    /**
       Array of pointers to pagetables.
    **/
    page_table_t *tables[1024];
    /**
       Array of pointers to the pagetables above, but gives their *physical*
       location, for loading into the CR3 register.
    **/
    uint32_t tablesPhysical[1024];

    /**
       The physical address of tablesPhysical. This comes into play
       when we get our kernel heap allocated and the directory
       may be in a different location in virtual memory.
    **/
    uint32_t physicalAddr;
} page_directory_t;

#define VMM_PAGE_SIZE 0x1000

void vmm_init();
void vmm_map_page(page_directory_t* dir, uint32_t vaddr, bool kernel, bool writeable);
void vmm_unmap_page(page_directory_t* dir, uint32_t vaddr);
uint32_t vmm_get_phys_addr(page_directory_t* dir, uint32_t vaddr);
page_directory_t* vmm_get_kdir();
page_directory_t* vmm_clone_dir(page_directory_t* src);
void vmm_destroy_dir(page_directory_t*);
void vmm_switch_dir(page_directory_t* new);
