#pragma once

#include <stdint.h>

struct chs_addr
{
	uint8_t head, sector, cylinder;
} __attribute__((packed));
typedef struct chs_addr chs_addr_t;

struct mbr_partition
{
	uint8_t status;
	chs_addr_t begin;
	uint8_t type;
	chs_addr_t end;
	uint32_t lba;
	uint32_t sectors;
} __attribute__((packed));
typedef struct mbr_partition mbr_partition_t;

struct mbr
{
	uint8_t code[446];
	mbr_partition_t partitions[4];
	uint16_t signature; // 0xAA55 in memory, 0x55AA on disk
}__attribute__((packed));
typedef struct mbr mbr_t;