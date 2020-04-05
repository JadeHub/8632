#pragma once

#include <kernel/fs/node.h>

fs_node_t* con_his_create(const char* con_name);
void con_his_destroy();
