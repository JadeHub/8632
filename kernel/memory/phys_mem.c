#include "phys_mem.h"
#include "paging.h"
#include "kmalloc.h"

#include <kernel/fault.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <kernel/debug.h>

uint32_t* _frames;
uint32_t _frame_cnt;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))


static uint32_t _test_frame(uint32_t frame_addr);

static void _set_frame(uint32_t frame_addr)
{
    ASSERT((frame_addr % 0x1000) == 0);
    if (frame_addr == 0)
    {
        printf("!!!!!!!!!!!!!!!!!!Allocating frame 0 0x%x 0x%x\n", _frames, _frames[0]);
        ASSERT(!_test_frame(frame_addr));
        bochs_dbg();
    }
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    _frames[idx] |= (0x1 << off);

    
}

static void _clear_frame(uint32_t frame_addr)
{
    ASSERT((frame_addr % 0x1000) == 0);
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    _frames[idx] &= ~(0x1 << off);

    if (idx == 0)
    {
       // dbg_dump_current_stack();
       printf("!!!!!!!!!!!!!!!!!!Clearing frame 0 0x%x 0x%x\n", _frames, _frames[0]);
        printf("frame_addr 0x%x frame 0x%x idx 0x%x off 0x%x\n", frame_addr, frame, idx, off);
        bochs_dbg();
    }
}

static uint32_t _test_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (_frames[idx] & (0x1 << off));
}

static uint32_t _first_frame()
{
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(_frame_cnt); i++)
    {
        if (_frames[i] != 0xFFFFFFFF)
        {
            for (j = 0; j < 32; j++)
            {
                if (!(_frames[i] & (0x1 << j)))
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
            KPANIC("No _frames left");
            // PANIC! no free _frames!!
        }
      //  else 
        ASSERT(!page->present);
        _set_frame(idx * 0x1000);
        page->present = 1;
        page->rw = (is_writeable) ? 1 : 0;
        page->user = (is_kernel) ? 0 : 1;
        page->frame = idx;
    }
}

void free_frame(page_t* page)
{
    ASSERT(page);
    if (page->frame < 0x100)
    {
        //kernel memory space
        KPANIC("Free < 0x100");
    }
    uint32_t frame = page->frame;
    if (frame == 0)
    {
        ASSERT(!page->present);
        return;
    }
    else
    {
        _clear_frame(frame*0x1000);
        page->frame = 0x0;
        page->present = 0;
    }
}

void phys_mem_init()
{
    //16mb mem
    uint32_t mem_end_page = 0x1000000;

    _frame_cnt = mem_end_page / 0x1000;
    _frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(_frame_cnt));
    memset(_frames, 0, INDEX_FROM_BIT(_frame_cnt));
}

/***************************************************************************************/


#define BYTE_FROM_FRAME(f__) (f__/32)
#define BIT_FROM_FRAME(f__) (f__%32)
#define FRAME_FROM_BYTE_BIT(byte__, bit__) ((byte__*32)+bit__)

uint32_t _frame_count = 0;
uint32_t _allocated_count = 0;
uint32_t* _frame_idx = NULL;
uint32_t _idx_size = 0; //number of bytes in the index

static bool _is_valid_frame(uint32_t frame)
{
    return BYTE_FROM_FRAME(frame) < _idx_size;
}

static uint32_t _is_frame_marked(uint32_t frame)
{
    return _frame_idx[BYTE_FROM_FRAME(frame)] & (1 << BIT_FROM_FRAME(frame));
}

static void _mark_frame(uint32_t frame, bool alloc)
{
    ASSERT(_is_valid_frame(frame));
    if (alloc)
    {
        ASSERT(!_is_frame_marked(frame));
        _frame_idx[BYTE_FROM_FRAME(frame)] |= (1 << BIT_FROM_FRAME(frame));
    }
    else
    {
        ASSERT(_is_frame_marked(frame));
        _frame_idx[BYTE_FROM_FRAME(frame)] &= ~(1 << BIT_FROM_FRAME(frame));        
    }
}

static uint32_t _first_unmarked()
{
    uint32_t i, j;
    for (i = 0; i < _idx_size; i++)
    {
        if (_frame_idx[i] != 0xFFFFFFFF)
        {
            for (j = 0; j < 32; j++)
            {
                if (!(_frame_idx[i] & (0x1 << j)))
                {
                    return FRAME_FROM_BYTE_BIT(i, j);
                }
            }
        }
    }
    return 0xFFFFFFFF;
}

void pmm_init(uint32_t mb)
{
    uint32_t size_in_bytes = mb * 0x100000;
    _frame_count = size_in_bytes / FRAME_SIZE;
    _idx_size = _frame_count / 8;
    _frame_idx = (uint32_t*)kmalloc(_idx_size);
}

uint32_t pmm_alloc_frame()
{
    uint32_t frame = _first_unmarked();
    if (frame == 0xFFFFFFFF)
        KPANIC("Out of memory");
    _mark_frame(frame, true);
    _allocated_count++;
    return frame;
}

void pmm_free_frame(uint32_t frame)
{
    _mark_frame(frame, false);
    _allocated_count--;
}