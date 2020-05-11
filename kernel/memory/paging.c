#include "paging.h"
#include "kheap.h"
#include "kmalloc.h"
#include "phys_mem.h"
#include <kernel/fault.h>
#include <kernel/debug.h>
#include <kernel/utils.h>
#include <kernel/x86/interrupts.h>
#include <kernel/tasks/sched.h>
#include <stdio.h>

extern void enable_paging();
extern void copy_page_physical(uint32_t, uint32_t);

#define INVALID_FRAME 0xFFFFF;
#define IS_PAGE_BOUNDRY(x) (x % VMM_PAGE_SIZE == 0)
#define PAGE_FROM_VADDR(v) (v / VMM_PAGE_SIZE)
#define TABLE_FROM_PAGE(p) (p / 1024)

static page_directory_t* _kpage_dir = NULL;
extern uint32_t placement_address;

extern heap_t *kheap;

static void page_fault(isr_state_t* regs)
{
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    // The error code gives us details of what happened.
    int present   = !(regs->err_code & 0x1); // Page not present
    int rw = regs->err_code & 0x2;           // Write operation?
    int us = regs->err_code & 0x4;           // Processor was in user-mode?
    int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = regs->err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    printf("Page fault! ( ");
    if (present) {printf("present ");}
    if (rw) {printf("read-only ");}
    if (us) {printf("user-mode ");}
    if (reserved) {printf("reserved ");}
    printf(") at 0x%08x - EIP: 0x%08x\n", faulting_address, regs->eip);

    dbg_dump_stack(us ? sched_cur_proc()->elf_img : dbg_kernel_image(), regs->ebp, regs->eip);
  //  KPANIC("Page fault");
    for (;;);
}

static page_table_t* _clone_table(page_table_t *src, uint32_t *physAddr)
{
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), physAddr);
	// Ensure that the new table is blank.
    memset(table, 0, sizeof(page_directory_t));

    // For every entry in the table...
    int i;
    for (i = 0; i < 1024; i++)
    {
        // If the source entry has a frame associated with it...
        if (!src->pages[i].frame)
            continue;
        // Get a new frame.
        alloc_frame(&table->pages[i], 0, 0);
        // Clone the flags from source to destination.
        if (src->pages[i].present) table->pages[i].present = 1;
        if (src->pages[i].rw)      table->pages[i].rw = 1;
        if (src->pages[i].user)    table->pages[i].user = 1;
        if (src->pages[i].accessed)table->pages[i].accessed = 1;
        if (src->pages[i].dirty)   table->pages[i].dirty = 1;

        // Physically copy the data across. This function is in process.s.
        copy_page_physical(src->pages[i].frame*0x1000, table->pages[i].frame*0x1000);
    }
    return table;
}

void vmm_destroy_dir(page_directory_t* dir)
{
    ASSERT(dir && dir != _kpage_dir);
    kfree(dir);
}

page_directory_t* vmm_clone_dir(page_directory_t *src)
{
    uint32_t phys;
    // Make a new page directory and obtain its physical address.
    page_directory_t *dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    // Ensure that it is blank.
    memset(dir, 0, sizeof(page_directory_t));

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    uint32_t offset = (uint32_t)dir->tablesPhysical - (uint32_t)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddr = phys + offset;

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!src->tables[i])
            continue;

        if (_kpage_dir->tables[i] == src->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        }
      /*  else
        {
            // Copy the table.
            uint32_t phys;
            dir->tables[i] = _clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }*/
    }
    return dir;
}

static page_t* _create_page_table(page_directory_t* dir, uint32_t vaddr)
{
    ASSERT(IS_PAGE_BOUNDRY(vaddr));
    uint32_t page_idx = PAGE_FROM_VADDR(vaddr);
    uint32_t table_idx = TABLE_FROM_PAGE(page_idx);

    if (!dir->tables[table_idx])
    {
        //make a new page table
        uint32_t phys_addr;
        dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &phys_addr);
        memset(dir->tables[table_idx], 0, 0x1000);
        for (uint32_t i = 0; i < 1024; i++)
            dir->tables[table_idx]->pages[i].frame = INVALID_FRAME;

        dir->tablesPhysical[table_idx] = phys_addr | 0x07; // PRESENT, RW, US. //todo ?
    }
    return &dir->tables[table_idx]->pages[page_idx % 1024];
}

page_t* vmm_get_page(page_directory_t* dir, uint32_t vaddr)
{
    uint32_t page = PAGE_FROM_VADDR(vaddr);
    uint32_t table = TABLE_FROM_PAGE(page);
    if (dir->tables[table])
        return &dir->tables[table]->pages[page % 1024];
    return NULL;
}

void vmm_unmap_page(page_directory_t* dir, uint32_t vaddr)
{
    ASSERT(IS_PAGE_BOUNDRY(vaddr));
    page_t* page = vmm_get_page(dir, vaddr);
    ASSERT(page && page->present);

    pmm_free_frame(page->frame);
    page->frame = INVALID_FRAME;
    page->present = 0;
}

void vmm_map_page(page_directory_t* dir, uint32_t vaddr, bool kernel, bool writeable)
{
    ASSERT(IS_PAGE_BOUNDRY(vaddr));
    uint32_t page_idx = PAGE_FROM_VADDR(vaddr);
    uint32_t table_idx = TABLE_FROM_PAGE(page_idx);
    if (!dir->tables[table_idx])
    {
        _create_page_table(dir, vaddr);
    }
    page_t* page = &dir->tables[table_idx]->pages[page_idx % 1024];
    ASSERT(!page->present);
    page->present = 1;
    page->rw = writeable ? 1 : 0;
    page->user = kernel ? 0 : 1;
    page->frame = pmm_alloc_frame();
}

void vmm_map_pages(page_directory_t* dir, uint32_t vaddr, uint32_t count, bool kernel, bool writeable)
{
    for (uint32_t i = 0; i < count; i++, vaddr += VMM_PAGE_SIZE)
    {
        vmm_map_page(dir, vaddr, kernel, writeable);
    }
}

uint32_t vmm_get_phys_addr(page_directory_t* dir, uint32_t vaddr)
{
    page_t* page = vmm_get_page(dir, vaddr);
    if (page && page->present)
        return page->frame * FRAME_SIZE + (vaddr & 0x00000FFF);
    return 0xFFFFFFFF;
}

page_directory_t* vmm_get_kdir()
{
    return _kpage_dir;
}

void vmm_switch_dir(page_directory_t* dir)
{
    asm volatile("mov %0, %%cr3":: "r"(dir->physicalAddr));
}

void vmm_init()
{
    uint32_t phys;
    _kpage_dir = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
    memset(_kpage_dir, 0, sizeof(page_directory_t));
    _kpage_dir->physicalAddr = (uint32_t)_kpage_dir->tablesPhysical;

    idt_register_handler(ISR14, &page_fault);

    //Map the first megabyte for use by the kernel
    for (uint32_t addr = 0; addr < 0x100000; addr += VMM_PAGE_SIZE)
    {
        // Kernel code is readable but not writeable from userspace.
        vmm_map_page(_kpage_dir, addr, false, false);
    }
    
    vmm_switch_dir(_kpage_dir);
    enable_paging();

    // Initialise the kernel heap.
    for (uint32_t addr = KHEAP_START; addr < KHEAP_START + KHEAP_INITIAL_SIZE; addr += VMM_PAGE_SIZE)
        vmm_map_page(_kpage_dir, addr, false, false);
    kheap = heap_create(_kpage_dir, KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, KHEAP_START + KHEAP_INITIAL_SIZE*2, 0, 0);
}
