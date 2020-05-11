#include "ramfs.h"

#include <kernel/vfs/vfs.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/fault.h>
#include <kernel/utils.h>

#include <string.h>
#include <stdio.h>

typedef struct header
{
	char path[128];
	uint32_t start;
	uint32_t len;
}header_t;

typedef struct ramdisk
{
	uint8_t* data;
	uint32_t len;
	uint32_t files;
	header_t* headers;

	fs_node_t* root_node;
}ramdisk_t;

static ramdisk_t _rd;
static uint32_t _next_inode = 0;

typedef struct
{
	list_head_t children;
}dir_t;


static uint32_t _fs_read_dir(fs_node_t* node, fs_read_dir_cb_fn_t cb, void* data)
{
	dir_t* dir = (dir_t*)node->data;
	fs_node_t* child;
	uint32_t count = 0;
	list_for_each_entry(child, &dir->children, list)
	{
		count++;
		if (!cb(node, child, data))
			break;
	}
	return count;
}

static fs_node_t* _fs_find_child(fs_node_t* node, const char* name)
{
	dir_t* dir = (dir_t*)node->data;
	fs_node_t* child;
	list_for_each_entry(child, &dir->children, list)
		if (strcmp(name, child->name) == 0)
			return child;
	return NULL;
}

static inline uint32_t _min(uint32_t a, uint32_t b)
{
	return a < b ? a : b;
}

static bool _open_file(fs_node_t* parent, fs_node_t* node)
{
	return true;
}

static size_t _read_file(struct fs_node* f, uint8_t* buff, size_t off, size_t sz)
{
	ASSERT(f && buff && sz);
	uint32_t file_offset = (uint32_t)f->data;
	uint32_t read_sz = _min(f->len, sz);

	memcpy(buff, _rd.data + file_offset + off, read_sz);
	return read_sz;
}

static fs_node_t* _create_dir_node(const char* name)
{
	fs_node_t* node = fs_create_node(name);
	node->inode = _next_inode++;
	node->flags |= FS_DIR;
	dir_t* dir = (dir_t*)kmalloc(sizeof(dir_t));
	INIT_LIST_HEAD(&dir->children);
	node->data = dir;
	node->read_dir = _fs_read_dir;
	node->find_child = _fs_find_child;
	return node;
}

static fs_node_t* _find_or_add_dir(fs_node_t* parent, char* name)
{
//	printf("Seeking %s in %s\n", name, parent->name);
	ASSERT(parent);
	fs_node_t* node = _fs_find_child(parent, name);
	if (node)
		return node;

	dir_t* parent_data = (dir_t*)parent->data;
	//printf("Creating %s in %s\n", name, parent->name);
	node = _create_dir_node(name);
	list_add(&node->list, &parent_data->children);
	return node;
	//return NULL;
	/*

	fs_node_t* child =  fs_find_child(parent, name);
	if(child) return child;

	//Create a dir node and add it to the parent
	child = fs_create_dir_node(name, _next_inode++);
	if (!fs_add_child_node(parent, child))
	{
		fs_destroy_node(child);
		child = NULL;
	}
	return child;*/
}

static fs_node_t* _add_file_index(fs_node_t* parent, char* path, uint32_t offset, uint32_t len)
{
	char* sep = strchr(path, '/');
	if (sep)
	{
		//Given "blah/blah2/file"
		*sep = '\0';
		sep++;
		//path = "blah"
		//sep = "blah2/file"
		return _add_file_index(_find_or_add_dir(parent, path), sep, offset, len);
	}
	else
	{
		fs_node_t* file = fs_create_node(path);
		file->len = len;
		file->inode = _next_inode++;
		file->data = (void*)offset;
		file->len = len;
		file->read = _read_file;
		file->open = _open_file;
		dir_t* data = (dir_t*)parent->data;
		list_add(&file->list, &data->children);
		return file;
	}
}

void ramfs_init(uint8_t* data, uint32_t len)
{
	memset(&_rd, 0, sizeof(ramdisk_t));
	_rd.data = (uint8_t*)kmalloc(len);
	memcpy(_rd.data, data, len);
	_rd.len = len;
	_rd.files = *(uint32_t*)_rd.data;
	_rd.headers = (header_t*)(_rd.data + sizeof(uint32_t));
	
	_rd.root_node = _create_dir_node("initrd");
	fs_install_root_fs(_rd.root_node);
	char buff[256];
	header_t* hdr = _rd.headers;
	for (int i = 0; i < _rd.files; i++, hdr++)
	{
		//_add_file_index will modify the path
		strcpy(buff, hdr->path);
		_add_file_index(_rd.root_node, buff, hdr->start, hdr->len);
	}
}

fs_node_t* ramfs_root()
{
	return _rd.root_node;
}