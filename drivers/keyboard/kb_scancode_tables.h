//#if 0
#pragma once

#include <stdint.h>

extern uint8_t kb_sc_ascii_lower[];
extern uint8_t kb_sc_ascii_upper[];
extern uint8_t kb_sc_ascii_2byte[];

void kb_scancode_tables_init();
//#endif