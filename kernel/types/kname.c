#include "kname.h"
#include <kernel/memory/kmalloc.h>
#include <string.h>
#include <stddef.h>

char* kname_set(const char* name, kname_t* kn)
{
	kname_destroy(kn);
	size_t len = strlen(name);
	if (len > KNAME_LEN - 1)
		kn->str = (char*)kmalloc(len + 1);
	else
		kn->str = kn->name_buff;
	strcpy(kn->str, name);
	return kn->str;
}

void kname_destroy(kname_t* kn)
{
	if (kn->str != kn->name_buff)
		kfree(kn->str);
	kn->str = NULL;
}