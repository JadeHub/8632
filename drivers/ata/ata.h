#pragma once

#include <stdint.h>

void ata_init();
void ata_read(uint8_t *buf, uint32_t lba, uint32_t numsects);
