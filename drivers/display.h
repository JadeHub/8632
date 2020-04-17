#pragma once

#include <stdint.h>

typedef enum
{
	BLACK			= 0,
	BLUE			= 1,
	GREEN			= 2,
	CYAN			= 3,
	RED				= 4,
	PURPLE			= 5,
	BROWN			= 6,
	GREY			= 7,
	DARK_GREY		= 8,
	LIGHT_BLUE		= 9,
	LIGHT_GREEN		= 10,
	LIGHT_CYAN		= 11,
	LIGHT_RED		= 12,
	LIGHT_PURPLE	= 13,
	YELLOW			= 14,
	WHITE			= 15
}dsp_color;

void dsp_print_char(uint8_t c, uint8_t col, uint8_t row);
void dsp_clear_screen();
void dsp_scroll(uint8_t lines);
void dsp_set_text_attr(dsp_color fore, dsp_color back);
void dsp_set_foreground(dsp_color);
void dsp_set_background(dsp_color);

uint8_t dsp_get_cursor_x();
uint8_t dsp_get_cursor_y();
void dsp_set_cursor(uint8_t col, uint8_t row);
void dsp_enable_cursor();
void dsp_disable_cursor();