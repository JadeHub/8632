#include "memory.h"
#include "console.h"

typedef struct
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t unused;
} bios_mem_info __attribute__((packed));

void dump(bios_mem_info* info)
{
    con_write("Base: ");
    con_write_hex(info->base >> 32);
    con_write(":");
    con_write_hex(info->base);
    con_write("\n");

    con_write("Len: ");
    con_write_hex(info->length >> 32);
    con_write(":");
    con_write_hex(info->length);
    con_write("\n");

    con_write("Type: ");
    con_write_hex(info->type);
    con_write("\n");
}

void mem_init()
{
    uint32_t len = *(uint32_t*)0x8000;
    bios_mem_info* mem_block = (bios_mem_info*)0x8004;

    con_write_hex(len);
    con_write("\n");

    for(uint32_t i=0; i<len;i++)
    {
        con_write("Block: ");
        con_write_hex(i);
        con_write("\n");

        dump(mem_block + i);        
    }
}