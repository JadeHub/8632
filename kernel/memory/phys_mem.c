#include "phys_mem.h"
#include "paging.h"
#include "kmalloc.h"

#include <kernel/fault.h>
#include <stdint.h>
#include <string.h>

uint32_t* frames;
uint32_t nframes;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void _set_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static void _clear_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

static uint32_t _test_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

static uint32_t _first_frame()
{
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
    {
        if (frames[i] != 0xFFFFFFFF)
        {
            for (j = 0; j < 32; j++)
            {
                uint32_t toTest = 0x1 << j;
                if (!(frames[i] & (0x1 << j)))
                {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
}

void alloc_frame(page_t* page, int is_kernel, int is_writeable)
{
    if (page->frame != 0)
    {
        return;
    }
    else
    {
        uint32_t idx = _first_frame();
        if (idx == (uint32_t)-1)
        {
            KPANIC("No frames left");
            // PANIC! no free frames!!
        }
        _set_frame(idx * 0x1000);
        page->present = 1;
        page->rw = (is_writeable) ? 1 : 0;
        page->user = (is_kernel) ? 0 : 1;
        page->frame = idx;
    }
}

void free_frame(page_t* page)
{
    uint32_t frame;
    if (!(frame = page->frame))
    {
        return;
    }
    else
    {
        _clear_frame(frame);
        page->frame = 0x0;
    }
}

void phys_mem_init()
{
    //16mb mem
    uint32_t mem_end_page = 0x1000000;

    nframes = mem_end_page / 0x1000;
    frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
    memset(frames, 0, INDEX_FROM_BIT(nframes));
}