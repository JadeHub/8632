#pragma once

#include <stdint.h>

typedef enum
{
	BLACK			= 1,
	BLUE			= 2,
	GREEN			= 3,
	CYAN			= 4,
	RED				= 5,
	PURPLE			= 6,
	BROWN			= 7,
	GREY			= 8,
	DARK_GREY		= 9,
	LIGHT_BLUE		= 10,
	LIGHT_GREEN		= 11,
	LIGHT_CYAN		= 12,
	LIGHT_RED		= 13,
	LIGHT_PURPLE	= 14,
	YELLOW			= 15,
	WHITE			= 16
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