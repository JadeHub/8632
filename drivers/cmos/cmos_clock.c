#include "cmos_clock.h"

#include <drivers/ioports.h>

#define CURRENT_YEAR 2020

int century_register = 0x00;                                // Set by ACPI table parsing code if possible

enum
{
    cmos_address = 0x70,
    cmos_data = 0x71
};

static int _get_update_in_progress_flag()
{
    outb(cmos_address, 0x0A);
    return (inb(cmos_data) & 0x80);
}

static unsigned char _get_RTC_register(int reg)
{
    outb(cmos_address, reg);
    return inb(cmos_data);
}

wall_clock_t cmos_read_clock()
{
    wall_clock_t result;
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates

    while (_get_update_in_progress_flag());                // Make sure an update isn't in progress
    result.second = _get_RTC_register(0x00);
    result.minute = _get_RTC_register(0x02);
    result.hour = _get_RTC_register(0x04);
    result.day = _get_RTC_register(0x07);
    result.month = _get_RTC_register(0x08);
    result.year = _get_RTC_register(0x09);
    if (century_register != 0)
    {
        century = _get_RTC_register(century_register);
    }

    do
    {
        last_second = result.second;
        last_minute = result.minute;
        last_hour = result.hour;
        last_day = result.day;
        last_month = result.month;
        last_year = result.year;
        last_century = century;

        while (_get_update_in_progress_flag());           // Make sure an update isn't in progress
        result.second = _get_RTC_register(0x00);
        result.minute = _get_RTC_register(0x02);
        result.hour = _get_RTC_register(0x04);
        result.day = _get_RTC_register(0x07);
        result.month = _get_RTC_register(0x08);
        result.year = _get_RTC_register(0x09);
        if (century_register != 0)
        {
            century = _get_RTC_register(century_register);
        }
    } while ((last_second != result.second) ||
        (last_minute != result.minute) ||
        (last_hour != result.hour) ||
        (last_day != result.day) ||
        (last_month != result.month) ||
        (last_year != result.year) ||
        (last_century != century));

    registerB = _get_RTC_register(0x0B);
    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04))
    {
        result.second = (result.second & 0x0F) + ((result.second / 16) * 10);
        result.minute = (result.minute & 0x0F) + ((result.minute / 16) * 10);
        result.hour = ((result.hour & 0x0F) + (((result.hour & 0x70) / 16) * 10)) | (result.hour & 0x80);
        result.day = (result.day & 0x0F) + ((result.day / 16) * 10);
        result.month = (result.month & 0x0F) + ((result.month / 16) * 10);
        result.year = (result.year & 0x0F) + ((result.year / 16) * 10);
        if (century_register != 0)
        {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    // Convert 12 result.hour clock to 24 result.hour clock if necessary
    if (!(registerB & 0x02) && (result.hour & 0x80))
    {
        result.hour = ((result.hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) result.year
    if (century_register != 0)
    {
        result.year += century * 100;
    }
    else
    {
        result.year += (CURRENT_YEAR / 100) * 100;
        if (result.year < CURRENT_YEAR) result.year += 100;
    }
    return result;
}