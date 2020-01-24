#pragma once

#define KNAME_LEN 128

typedef struct kname
{
	char name_buff[KNAME_LEN];
	char* name;
}kname_t;

char* kname_create(const char* name, kname_t* kn);
void kname_destroy(kname_t* name);