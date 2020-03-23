#include "display.h"
#include "ioports.h"

#include <string.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static uint16_t* video_memory = (uint16_t *) 0xb8000;
static uint16_t text_attr = (1 << 12) | (15 << 8);

static uint16_t get_char_offset(uint8_t col, uint8_t row)
{
	return row * SCREEN_WIDTH + col;
}

void dsp_set_text_attr(uint8_t fore, uint8_t back)
{
	text_attr = (back << 12) | (fore << 8);
}

void dsp_print_char(uint8_t c, uint8_t col, uint8_t row)
{
	video_memory[get_char_offset(col, row)] = c | text_attr;
}

void dsp_remove_scroll(uint8_t col, uint8_t row)
{
	for (uint8_t copy = col; copy < SCREEN_WIDTH - 1; copy++)
		video_memory[get_char_offset(copy, row)] = video_memory[get_char_offset(copy+1, row)];
	video_memory[get_char_offset(SCREEN_WIDTH - 1, row)] = ' ' | text_attr;
}

void dsp_scroll_char(uint8_t c, uint8_t col, uint8_t row)
{
	for (uint8_t copy = SCREEN_WIDTH - 1; copy > col; copy--)
		video_memory[get_char_offset(copy, row)] = video_memory[get_char_offset(copy-1, row)];

	video_memory[get_char_offset(col, row)] = c | text_attr;
}

void dsp_clear_screen()
{
	for (uint8_t col=0; col<SCREEN_WIDTH; col++)
		for(uint8_t row=0; row<SCREEN_HEIGHT; row++)
			dsp_print_char(' ', col, row);
}

void dsp_scroll(uint8_t lines)
{
	while (lines)
	{
		int i;
		for (i = 0; i < 24 * 80; i++)
			video_memory[i] = video_memory[i+80];
		for (i = 24*80; i < 25*80; i++)
			video_memory[i] = ' ' | text_attr;
		lines--;	
	}
}

static int COMMAND_PORT = 0x3D4;
static int DATA_PORT = 0x3D5;

void dsp_set_cursor(uint8_t col, uint8_t row)
{
	uint16_t cursorLocation = get_char_offset(col, row);
	outb(COMMAND_PORT, 14);		// setting high cursor byte
	outb(DATA_PORT, cursorLocation >> 8);
	outb(COMMAND_PORT, 15);		// setting low cursor byte
	outb(DATA_PORT, cursorLocation);
}

void dsp_enable_cursor()
{
	outb(COMMAND_PORT, 0x0A);
	outb(DATA_PORT, (inb(DATA_PORT) & 0xC0) | 12);
 
	outb(COMMAND_PORT, 0x0B);
	outb(DATA_PORT, (inb(DATA_PORT) & 0xE0) | 15);
}

void dsp_disable_cursor()
{
	outb(COMMAND_PORT, 0x0A);
	outb(DATA_PORT, 0x20);
}
