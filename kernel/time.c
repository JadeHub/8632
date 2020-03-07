#include "time.h"

#include <drivers/timer/timer.h>
#include <drivers/cmos/cmos_clock.h>

#include <stdio.h>

uint64_t time_ticks()
{
	uint64_t ticks;
	__asm__ __volatile__("rdtsc":"=A"(ticks));
	return ticks;
}

void time_init()
{
	wall_clock_t c = cmos_read_clock();
	time_print_clock(&c);
}

void time_on_tick(uint64_t ms)
{
	if (ms % 1000 == 0)
	{
		wall_clock_t c = cmos_read_clock();
		time_print_clock(&c);
	}
}

void time_print_clock(wall_clock_t* wc)
{
	printf("%02d:%02d:%02d %02d/%02d/%d\n",
		wc->hour, wc->minute, wc->second, wc->day, wc->month, wc->year);
}