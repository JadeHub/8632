#pragma once

#include <kernel/fs/node.h>
#include <kernel/types/list.h>

typedef struct dirent
{
	fs_node_t* node;
	list_head_t list;		//this item's entry in its parent's list
	list_head_t child_list; //this item's children
}dirent_t;

/*
Create a dir node with name. Destory with fs_destroy_node(...)
*/
fs_node_t* fs_create_dir_node(char* name, uint32_t inode);