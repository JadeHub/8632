#include "ramfs.h"

#include <kernel/fs/fs.h>
#include <kernel/fs/dir.h>
#include <kernel/memory/kmalloc.h>

#include <drivers/console.h>

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
		// blah/blah2/file
		*sep = '\0';
		sep++;
		return _add_file_index(_find_or_add_dir(parent, path), sep, offset, len);
	}
	else
	{
		fs_node_t* file = fs_create_node(path);
		file->len = len;
		file->inode = _next_inode++;
		file->data = (void*)offset;
		file->len = len;
		if(fs_add_child_node(parent, file) != file)
		{
			fs_destroy_node(file);
			file = NULL;
		}
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

	_rd.root_node = fs_create_dir_node("initrd", _next_inode++);

	char buff[256];
	header_t* hdr = _rd.headers;
	for (int i = 0; i < _rd.files; i++, hdr++)
	{
		//_add_file_index will modify the path
		strcpy(buff, hdr->path);
		_add_file_index(_rd.root_node, buff, hdr->start, hdr->len);
	}

	fs_read_dir(_rd.root_node, &test);
}

fs_node_t* ramfs_root()
{
	return _rd.root_node;
}