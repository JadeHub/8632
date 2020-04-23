#pragma once

#include <sys/cdefs.h>
#include <sys/signal_defs.h>

#include <dirent.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

__LIBC_BEGIN_H

void* sys_alloc(size_t);
void sys_print_str(const char*, uint32_t);
void sys_exit(int32_t);
uint32_t sys_open(const char*, uint32_t);
size_t sys_read(uint32_t, uint8_t*, size_t);
size_t sys_write(uint32_t, const uint8_t*, size_t);
void sys_close(uint32_t);
void sys_sleep_ms(uint32_t);
void sys_read_dir(const char* path, void(fn)(const char*));
struct DIR* sys_opendir(const char*);
void sys_closedir(struct DIR*);
bool sys_readdir(struct DIR*, struct dirent*);
uint32_t sys_start_proc(const char* path, const char* args[], uint32_t fds[3]);
uint32_t sys_wait_pid(uint32_t);
void sys_reg_sig_handler(int, sig_handler_t);
void sys_sig_handler_return();
bool sys_send_signal(uint32_t pid, uint32_t sig);

__LIBC_END_H