#include "kname.h"
#include <kernel/memory/kmalloc.h>
#include <string.h>
#include <stddef.h>

char* kname_create(const char* name, kname_t* kn)
{
	size_t len = strlen(name);
	if (len > KNAME_LEN - 1)
		kn->name = (char*)kmalloc(len + 1);
	else
		kn->name = kn->name_buff;
	strcpy(kn->name, name);
	return kn->name;
}

void kname_destroy(kname_t* kn)
{
	if (kn->name != kn->name_buff)
		kfree(kn->name);
	kn->name = NULL;
}