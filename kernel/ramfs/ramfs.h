#pragma once

#include <kernel/vfs/node.h>

#include <stdint.h>

void ramfs_init(uint8_t* data, uint32_t len);
fs_node_t* ramfs_root();