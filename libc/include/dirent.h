#pragma once

#include <sys/cdefs.h>

#include <stdint.h>

__LIBC_BEGIN_H

//types
#define DT_UNKNOWN	0
#define DT_REG 1
#define DT_DIR 2
#define DT_LNK 3

typedef struct dirent
{
	char name[256];
	uint8_t type;
	uint64_t size;
}dirent_t;

struct DIR* opendir(const char*);
void closedir(struct DIR*);
struct dirent* readdir(struct DIR*);

__LIBC_END_H