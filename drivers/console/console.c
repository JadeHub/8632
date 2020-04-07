#include "console.h"
#include "line_buff.h"
#include "history.h"

#include <drivers/display.h>
#include <drivers/ioports.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/device_driver.h>
#include <kernel/fault.h>
#include <kernel/io/io.h>
#include <kernel/fs/fs.h>

#include <kernel/types/cbuff.h>

#include <string.h>
#include <stdio.h>

static char _input_buff[1024];

typedef struct console
{
    dev_device_t device;
    uint8_t cursor_x;
    uint8_t cursor_y;

    //input buffer for kb isr
    line_buff_t line_buff;

    //stream buffer for clients to read from
    cbuff8_t* input_buff;

    con_history_t* history;

    bool in_esc;
    char esc_buff[16];
}console_t;

static dev_driver_t _driver;
static console_t _console;

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static void _replace_str(line_buff_t* lb, uint8_t start_x, size_t prev_len)
{
    uint8_t xcur = start_x;
    for (size_t x = 0; x < prev_len; x++)
        dsp_print_char(' ', xcur++, _console.cursor_y);

    xcur = start_x;
    size_t pos = 0;
    for (pos = 0; pos < lb->len; pos++)
        dsp_print_char(lb->result[pos], xcur++, _console.cursor_y);
}

static bool _is_ctrl_and(kb_event_t* kbe, uint8_t code)
{
    return kbe->code == code && kb_is_ctrl(&kbe->state);
}

static void _on_kb_event(kb_event_t* kbe)
{
    if (kbe->pressed)
    {
        size_t cur_len = _console.line_buff.len;
        if (cur_len == 0)
            _console.line_buff.cursor_start_x = _console.cursor_x;

        if (kb_is_ctrl(kbe))
        {
            if (kbe->code == 'c')
            {
                //send sigterm
            }
            return;
        }

        lb_result_t result = lb_add_code(&_console.line_buff, kbe->code, _console.history);
        if (result == BREAK)
        {
            con_putc('\n');
            con_his_add(_console.history, _console.line_buff.result);
            //Copy the line from the line_buff to the input stream buffer
            for (size_t i = 0; i < _console.line_buff.len; i++)
                cbuff8_put(_console.input_buff, _console.line_buff.result[i]);
            cbuff8_put(_console.input_buff, '\n');
            lb_init(&_console.line_buff);
            dev_unblock_readers(&_console.device);
            return;
        }
        else if (result == REPLACE)
        {
            _replace_str(&_console.line_buff, _console.line_buff.cursor_start_x, cur_len);
        }
        else if (result == APPEND)
        {
            dsp_print_char(kbe->code, _console.cursor_x, _console.cursor_y);
        }
        _console.cursor_x = _console.line_buff.cursor_start_x + _console.line_buff.cur_pos;
        dsp_set_cursor(_console.cursor_x, _console.cursor_y);
    }
}

static int32_t _open_con(dev_device_t* d, uint32_t flags)
{
    kb_register_event_handler(_on_kb_event);
    _console.history = con_his_create();
    return 0;
}

static void _close_con(dev_device_t* d)
{
    kb_remove_event_handler();
    con_his_destroy(_console.history);
}

static size_t _read_con(dev_device_t* d, uint8_t* buff, size_t off, size_t sz)
{
    size_t i;
    for (i = 0; i < sz; i++)
    {
        uint8_t val;

        while (!cbuff8_get(_console.input_buff, &val))
            dev_block_until_read(&_console.device);
        buff[i] = val;
    }
    return i;
}

static size_t _write_con(dev_device_t* d, const uint8_t* buff, size_t off, size_t sz)
{
    for (size_t i = 0; i < sz; i++)
        con_putc(buff[i]);
    return sz;
}

static void _append_char(char* str, char c)
{
    while ((*str) != '\0')
        str++;
    (*str++) = c;
    (*str) = '\0';
}

static bool _process_esc_sequence(const char* str)
{
    if (strcmp(str, "c") == 0)
    {
        con_clear();
        return true;
    }
    return false;
}

static bool _handle_esc_char(uint8_t c)
{
    if (c == KEY_ESC)
    {
        ASSERT(!_console.in_esc);
        _console.in_esc = true;
        _console.esc_buff[0] = '\0';
        return true;
    }

    if (_console.in_esc)
    {
        _append_char(_console.esc_buff, (char)c);
        if (_process_esc_sequence(_console.esc_buff))
            _console.in_esc = false;
        return true;
    }
    return false;
}

void con_init()
{
    memset(&_console, 0, sizeof(console_t));
    _console.input_buff = cbuff8_create(_input_buff, 1024);
    dsp_clear_screen();
    dsp_set_cursor(_console.cursor_x, _console.cursor_y);
}

void con_dev_init()
{
    memset(&_driver, 0, sizeof(dev_driver_t));
    kname_set("console_driver", &_driver.name);
    kname_set("console", &_driver.device_subdir);
    kname_set("con1", &_console.device.name);
    _driver.read = &_read_con;
    _driver.write = &_write_con;
    _driver.open = &_open_con;
    _driver.close = &_close_con;
    _console.device.driver = &_driver;

    dev_install_driver(&_driver);
    dev_register_device(&_console.device);    
}

void con_putc(uint8_t c)
{
    if (_handle_esc_char(c))
    {
        return;
    }
    else if(c == KEY_TAB)
    {
        _console.cursor_x += 4;
    }
    else if(c == '\r')
    {
        _console.cursor_x = 0;
    }
    else if(c == '\n')
    {
        _console.cursor_x = 0;
        _console.cursor_y++;
    }
    else
    {
        dsp_print_char(c, _console.cursor_x, _console.cursor_y);
        _console.cursor_x++;
    }    

    if(_console.cursor_x >= SCREEN_WIDTH)
    {
        _console.cursor_x = 0;
        _console.cursor_y++;
    }
    if (_console.cursor_y == SCREEN_HEIGHT)
    {
        dsp_scroll(1);
        _console.cursor_y = SCREEN_HEIGHT-1;
    }
    dsp_set_cursor(_console.cursor_x, _console.cursor_y);
}

void con_write_buff(const uint8_t* buff, size_t sz)
{
    for (size_t i = 0; i < sz; i++)
        con_putc(buff[i]);
}

void con_clear()
{
    dsp_clear_screen();
    _console.cursor_x = _console.cursor_y = 0;
    dsp_set_cursor(_console.cursor_x, _console.cursor_y);
}