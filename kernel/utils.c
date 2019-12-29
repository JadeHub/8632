#include "utils.h"

#include <kernel/memory/kmalloc.h>

#include <ctype.h>

#include <stddef.h>


char* copy_str(const char* str)
{
	char* r = (char*)kmalloc(strlen(str)+1);
	strcpy(r, str);
	return r;
}

