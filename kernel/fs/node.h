#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct fs_node;

typedef void (*fs_open_fn_t)(struct fs_node*, uint32_t flags); //return fd?
typedef void (*fs_close_fn_t)(struct fs_node*);
typedef size_t (*fs_read_fn_t)(struct fs_node*, uint8_t* buff, size_t off, size_t sz);
typedef size_t (*fs_write_fn_t)(struct fs_node*, uint8_t* buff, size_t off, size_t sz);
typedef bool(*fs_read_dir_cb_fn_t)(struct fs_node* parent, struct fs_node* child);
typedef uint32_t (*fs_read_dir_fn_t)(struct fs_node*, fs_read_dir_cb_fn_t);
typedef struct fs_node* (*fs_find_child_fn_t)(struct fs_node*, const char* name); //find a child by name
typedef struct fs_node* (*fs_add_child_fn_t)(struct fs_node*, struct fs_node*);
typedef bool (*fs_remove_child_fn_t)(struct fs_node*, struct fs_node*);

typedef struct fs_node
{
	char name_buff[64];
	char* name;
	uint32_t len; //0 for directories
	uint32_t flags;
	uint32_t inode; //unique id for this node
	void* data; //fs driver data
	struct fs_node* link;

	fs_open_fn_t open;
	fs_close_fn_t close;
	fs_read_fn_t read;
	fs_write_fn_t write;
	fs_read_dir_fn_t read_dir;
	fs_find_child_fn_t find_child;	
	fs_add_child_fn_t add_child;
	fs_remove_child_fn_t remove_child;
}fs_node_t;

#define FS_FILE		1
#define FS_DIR		2
//sym link, mount, char/block dev...

fs_node_t* fs_create_node(const char* name);
void fs_destroy_node(fs_node_t*);

