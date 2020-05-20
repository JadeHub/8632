#include "klog.h"

#include <drivers/cmos/cmos_clock.h>

#include <kernel/debug.h>
#include <kernel/time.h>
#include <kernel/io/io.h>
#include <kernel/fault.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/time.h>

#include <stdio.h>

static const char* _LM = "SYSTEM";

#define INIT_BUFF_ENTRIES 256

static char* _initial_buff[256];
static fd_t _persistence = INVALID_FD;
static wall_clock_t _start_time;

static void _write(const char* msg)
{
    ASSERT(_persistence != INVALID_FD);
    io_write(_persistence, msg, strlen(msg));
    io_write(_persistence, "\n", 1);
    io_flush(_persistence);
}

static void _persist_cache()
{
    for (uint32_t i = 0; i < INIT_BUFF_ENTRIES; i++)
    {
        if (_initial_buff[i] != NULL)
        {

            _write(_initial_buff[i]);
            //these will have been allocated before the heap was created so we don't attempt to kfree() them
            kfree(_initial_buff[i]);
            _initial_buff[i] = NULL;
        }
    }
}

static void _cache(const char* msg)
{
    for (uint32_t i = 0; i < INIT_BUFF_ENTRIES; i++)
    {
        if (_initial_buff[i] == NULL)
        {
            _initial_buff[i] = (char*)kmalloc(strlen(msg) + 1);
            strcpy(_initial_buff[i], msg);
            return;
        }
    }
    //out of initial buffer space
    bochs_dbg();
}

void klog_init()
{
    _start_time = cmos_read_clock();
    for (uint32_t i = 0; i < INIT_BUFF_ENTRIES; i++)
    {
        _initial_buff[i] = NULL;
    }
    KLOG(LL_INFO, "Startup. date:%02d-%02d-%04d time:%02d:%02d:%02d",
        _start_time.year, _start_time.month, _start_time.day,
        _start_time.hour, _start_time.minute, _start_time.second);
}

void klog_init_persistence()
{
    char path[256];
    wall_clock_t wc = cmos_read_clock();

    sprintf(path, "/fatfs/log/%04d%02d%02d-%02d%02d.log",
        wc.year, wc.month, wc.day, wc.hour, wc.minute);

    uint32_t tries = 0;
    while (io_exists(path))
    {
        if ((++tries) >= 100)
            break;
        sprintf(path, "/fatfs/log/%04d%02d%02d-%02d%02d-%02d.log",
            wc.year, wc.month, wc.day, wc.hour, wc.minute, tries);
    }
    _persistence = io_open(path, OPEN_CREATE | OPEN_KFILE);
    ASSERT(_persistence != INVALID_FD);
    _persist_cache();
}

void klog_impl(const char* file, uint32_t line, KLOG_LEVEL level, const char* module, const char* format, ...)
{
    char message[LOG_MAX_LINE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, LOG_MAX_LINE, format, args);
    va_end(args);

    char full_line[LOG_MAX_LINE];

    snprintf(full_line, LOG_MAX_LINE, "%08lld [%-6s] %s", time_ms(), module, message);

    if (_persistence != INVALID_FD)
        _write(full_line);
    else
        _cache(full_line);

    if (level == LL_ERR)
    {
        printf(full_line);
        bochs_dbg();
    }
}