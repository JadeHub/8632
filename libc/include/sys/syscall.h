#pragma once

#include <stdint.h>
#include <stddef.h>

void* sys_alloc(size_t);
void sys_print_str(const char*, uint32_t);
void sys_exit(uint32_t);
uint32_t sys_open(const char*, uint32_t);
size_t sys_read(uint32_t, uint8_t*, size_t);
void sys_close(uint32_t);
void sys_sleep_ms(uint32_t);
void sys_read_dir(const char* path, void(fn)(const char*));

struct DIR* sys_opendir(const char*);
void sys_closedir(struct DIR*);
struct dirent* sys_readdir(struct DIR*);