#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <kernel/types/list.h>

struct fs_node;

//file io
typedef int32_t (*fs_open_fn_t)(struct fs_node* parent, struct fs_node* node, uint32_t flags);
typedef void (*fs_close_fn_t)(struct fs_node*);
typedef size_t (*fs_read_fn_t)(struct fs_node*, uint8_t* buff, size_t off, size_t sz);
typedef size_t (*fs_write_fn_t)(struct fs_node*, const uint8_t* buff, size_t off, size_t sz);
typedef bool (*fs_remove_fn_t)(struct fs_node*);
//dir related
typedef bool(*fs_read_dir_cb_fn_t)(struct fs_node* parent, struct fs_node* child, void*);
typedef uint32_t (*fs_read_dir_fn_t)(struct fs_node*, fs_read_dir_cb_fn_t, void*);
typedef struct fs_node* (*fs_find_child_fn_t)(struct fs_node*, const char* name); //find a child by name

typedef struct fs_node
{
	char name_buff[64];
	char* name;
	uint32_t len; //0 for directories
	uint32_t flags;
	uint32_t inode; //unique id for this node
	void* data; //fs driver data
	struct fs_node* link;

	list_head_t list;

	fs_open_fn_t open;
	fs_close_fn_t close;
	fs_read_fn_t read;
	fs_write_fn_t write;
	fs_remove_fn_t remove;
	fs_read_dir_fn_t read_dir;
	fs_find_child_fn_t find_child;	
}fs_node_t;

//node flag values
#define FS_FILE		1
#define FS_DIR		2
//sym link, mount, char/block dev...

fs_node_t* fs_create_node(const char* name);
void fs_destroy_node(fs_node_t*);
bool fs_is_dir(const fs_node_t*);
bool fs_is_link(const fs_node_t*);

