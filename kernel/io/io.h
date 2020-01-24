#pragma once

#include <stdint.h>

uint32_t open(const char* path, uint32_t flags);
void close(uint32_t file);
size_t read(uint32_t file, uint8_t* buff, size_t len);
size_t write(uint32_t file, const uint8_t* buff, size_t len);

void io_init();