#pragma once

#include <kernel/vfs/node.h>

#define FS_MAX_PATH 1024

void fs_init();

fs_node_t* fs_root();
fs_node_t* fs_install_root_fs(fs_node_t* n);

fs_node_t* fs_get_abs_path(const char* path, fs_node_t** parent);

int32_t fs_open(fs_node_t*, uint32_t flags);
void fs_close(fs_node_t*);
size_t fs_read(fs_node_t*, uint8_t* buff, size_t off, size_t sz);
size_t fs_write(fs_node_t* node, const uint8_t* buff, size_t off, size_t sz);
uint32_t fs_read_dir(fs_node_t*, fs_read_dir_cb_fn_t, void*);
fs_node_t* fs_find_child(fs_node_t*, const char* name);
