#include "ramfs.h"

#include <kernel/vfs/vfs.h>
#include <kernel/vfs/dir.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/fault.h>
#include <kernel/utils.h>

#include <string.h>

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

static inline uint32_t _min(uint32_t a, uint32_t b)
{
	return a < b ? a : b;
}

static size_t _read_file(struct fs_node* f, uint8_t* buff, size_t off, size_t sz)
{
	ASSERT(f && buff && sz);
	uint32_t file_offset = (uint32_t)f->data;
	uint32_t read_sz = _min(f->len, sz);

	memcpy(buff, _rd.data + file_offset + off, read_sz);
	return read_sz;
}

static fs_node_t* _find_or_add_dir(fs_node_t* parent, char* name)
{
	fs_node_t* child =  fs_find_child(parent, name);
	if(child) return child;

	//Create a dir node and add it to the parent
	child = fs_create_dir_node(name, _next_inode++);
	if (!fs_add_child_node(parent, child))
	{
		fs_destroy_node(child);
		child = NULL;
	}
	return child;
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
		file->read = &_read_file;
		if(fs_add_child_node(parent, file) != file)
		{
			fs_destroy_node(file);
			file = NULL;
		}
		return file;
	}
}

static bool _remove_dir_fns_cb(fs_node_t* parent, fs_node_t* child, void* data)
{
	if (child->flags && FS_DIR)
	{
		child->add_child = NULL;
		child->remove_child = NULL;
	}
	return true;
}

void ramfs_init(uint8_t* data, uint32_t len)
{
	memset(&_rd, 0, sizeof(ramdisk_t));
	_rd.data = (uint8_t*)kmalloc(len);
	memcpy(_rd.data, data, len);
	_rd.len = len;
	_rd.files = *(uint32_t*)_rd.data;
	_rd.headers = (header_t*)(_rd.data + sizeof(uint32_t));
		
	_rd.root_node = fs_create_dir_node("initrd", _next_inode++);
	fs_install_root_fs(_rd.root_node);
	char buff[256];
	header_t* hdr = _rd.headers;
	for (int i = 0; i < _rd.files; i++, hdr++)
	{
		//_add_file_index will modify the path
		strcpy(buff, hdr->path);
		_add_file_index(_rd.root_node, buff, hdr->start, hdr->len);
	}
	//remove the directory modificantion fns as we're read only
	fs_walk_dir(_rd.root_node, &_remove_dir_fns_cb, 0);
}

fs_node_t* ramfs_root()
{
	return _rd.root_node;
}