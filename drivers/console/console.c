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

#include <string.h>

typedef struct console
{
    dev_device_t device;
    uint8_t cursor_x;
    uint8_t cursor_y;

    uint32_t kbd_fd;
    con_history_t* history;

    bool in_esc;
    char esc_buff[16];
}console_t;

static dev_driver_t _driver;
static console_t _console;

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static int32_t _open_con(dev_device_t* d, uint32_t flags)
{
     _console.kbd_fd = io_open("/dev/keyboard", 0);
    ASSERT(_console.kbd_fd != 0xffffffff);
    _console.history = con_his_create();
    return 0;
}

static void _close_con(dev_device_t* d)
{
    con_his_destroy(_console.history);
    io_close(_console.kbd_fd);
}

static void _erase(uint8_t cursor_start_x, size_t prev_len)
{
    uint8_t xcur = cursor_start_x;
    for (size_t x=0; x < prev_len; x++)
        dsp_print_char(' ', xcur++, _console.cursor_y);
}

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

static bool _is_ctrl_c(kb_event_t* kbe)
{
    return (kbe->code == 'c' || kbe->code == 'C') && kb_is_ctrl(&kbe->state);
}

static size_t _read_con(dev_device_t* d, uint8_t* buff, size_t off, size_t sz)
{
    line_buff_t lb;
    kb_event_t kbe;

    lb_init(&lb);
    uint8_t cursor_start_x = _console.cursor_x;
    while(lb.len < sz)
    {
       io_read(_console.kbd_fd, (uint8_t*)&kbe, sizeof(kb_event_t));
       if (kbe.pressed)
       {
           if (_is_ctrl_c(&kbe))
           {
               con_putc('^');
               con_putc('c');
               con_putc('\n');
               buff[0] = '\0';
               return 0;
           }

           if (kb_is_ctrl(&kbe.state) || kb_is_alt(&kbe.state))
           {
               continue;
           }

           size_t cur_len = lb.len;
           lb_result_t result = lb_add_code(&lb, kbe.code, _console.history);
           if (result == BREAK)
               break;
           else if (result == REPLACE)
               _replace_str(&lb, cursor_start_x, cur_len);
           else if (result == APPEND)
               dsp_print_char(kbe.code, _console.cursor_x, _console.cursor_y);
           _console.cursor_x = cursor_start_x + lb.cur_pos;
           dsp_set_cursor(_console.cursor_x, _console.cursor_y);
       }
    }
    con_putc('\n');
    strcpy(buff, lb.result);
    if (strlen(buff) > 0)
        con_his_add(_console.history, buff);
    return strlen(buff);
}

static size_t _write_con(dev_device_t* d, const uint8_t* buff, size_t off, size_t sz)
{
    for (size_t i = off; i < (off + sz); i++)
    {
        con_putc(buff[i]);
    }
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