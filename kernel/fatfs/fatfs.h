#pragma once

#include <stdint.h>

void fatfs_mount_partition(uint8_t ide_controller, uint8_t drive, uint8_t partition);