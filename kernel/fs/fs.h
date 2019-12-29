#pragma once

#include <kernel/fs/node.h>

void fs_init();

fs_node_t* fs_root();

void fs_open(fs_node_t*, uint32_t flags);
void fs_close(fs_node_t*);
size_t fs_read(fs_node_t*, uint8_t* buff, size_t off, size_t sz);
size_t fs_write(fs_node_t* node, uint8_t* buff, size_t off, size_t sz);
uint32_t fs_read_dir(fs_node_t*, fs_read_dir_cb_fn_t);
fs_node_t* fs_find_child(fs_node_t*, const char* name);
fs_node_t* fs_add_child_node(fs_node_t*, fs_node_t*);
bool fs_remove_child_node(fs_node_t*, fs_node_t*);