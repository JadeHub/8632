#pragma once

#include <stdint.h>

void dsp_print_char(uint8_t c, uint8_t col, uint8_t row);
void dsp_scroll_char(uint8_t c, uint8_t col, uint8_t row);
void dsp_remove_scroll(uint8_t col, uint8_t row);
void dsp_clear_screen();
void dsp_scroll(uint8_t lines);
void dsp_set_text_attr(uint8_t fore, uint8_t back);

uint8_t dsp_get_cursor_x();
uint8_t dsp_get_cursor_y();
void dsp_set_cursor(uint8_t col, uint8_t row);
void dsp_enable_cursor();
void dsp_disable_cursor();