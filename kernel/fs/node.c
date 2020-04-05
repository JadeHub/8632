#include "node.h"

#include <kernel/memory/kmalloc.h>

#include "string.h"

static bool _is_dir(fs_node_t* n)
{
	return n->flags & FS_DIR;
}

fs_node_t* fs_create_node(const char* name)
{
	fs_node_t* n = (fs_node_t*)kmalloc(sizeof(fs_node_t));
	memset(n, 0, sizeof(fs_node_t));

	if (strlen(name) < 64)
		n->name = n->name_buff;
	else
		n->name = (char*)kmalloc(strlen(name) + 1);
	strcpy(n->name_buff, name);

	return n;
}

void fs_destroy_node(fs_node_t* n)
{
	if (n->name != n->name_buff)
		kfree(n->name);
	kfree(n);
}

bool fs_is_dir(const fs_node_t* node)
{
	return node->flags & FS_DIR;
}
bool fs_is_link(const fs_node_t* node)
{
	return node->link != NULL;
}


