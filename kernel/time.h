#pragma once

#include <stdint.h>

typedef struct
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint32_t year;
}wall_clock_t;

void time_init();
uint64_t time_ticks();
void time_print_clock(wall_clock_t* wc);

void time_on_tick(uint64_t ms);