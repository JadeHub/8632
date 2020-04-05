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
    fs_node_t* history_node;
//    uint32_t history_fd;
}console_t;

static dev_driver_t _driver;
static console_t _console;

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static int32_t _open_con(dev_device_t* d, uint32_t flags)
{
     _console.kbd_fd = io_open("/dev/keyboard", 0);
    ASSERT(_console.kbd_fd != 0xffffffff);
    con_his_create("con1");
    _console.history_node = fs_get_abs_path("/dev/console/con1_history", NULL);
    ASSERT(_console.history_node != NULL);
    return 0;
}

static void _close_con(dev_device_t* d)
{
    con_his_destroy();
    io_close(_console.kbd_fd);
}

static void _erase(line_buff_t* lb, uint8_t cursor_start_x)
{
    _console.cursor_x = cursor_start_x;
    uint8_t xcur = cursor_start_x;
    for (size_t x=0; x < strlen(lb->result); x++)
        dsp_print_char(' ', xcur++, _console.cursor_y);
}

static void _replace_str(line_buff_t* lb, uint8_t cursor_start_x)
{
    _console.cursor_x = cursor_start_x;
    size_t pos = 0;
    for (pos = 0; pos < lb->len; pos++)
        con_putc(lb->result[pos]);
}

static bool _requires_reaplce(uint8_t code)
{
    return code == KEY_UP ||
        code == KEY_DOWN ||
        code == KEY_DEL ||
        code == KEY_BACKSPACE;
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
        /*   if (kbe.code == KEY_UP)
           {
               char prev[128];
              // strcpy(prev, "test1");
               fs_read(_console.history_node, prev, 0, 128);
               _erase(&lb, cursor_start_x);
               lb_set(&lb, prev);
               _replace_str(&lb, cursor_start_x);
               continue;
           }*/
         /*  else if (kbe.code == KEY_DEL || kbe.code == KEY_BACKSPACE)
           {
               lb_add_code(&lb, kbe.code);
               lb_set(lb, lb.result);
               continue;
           }*/

           uint8_t ascii = lb_add_code(&lb, kbe.code, _console.history_node);
           if (ascii == 0xFF)
               break;
           if (ascii)
           {
               con_putc(ascii);
           }
       }
    }
    con_putc('\n');
   //  printf("LF result %d\n", strlen(lb.result));
    strcpy(buff, lb.result);
    fs_write(_console.history_node, buff, 0, strlen(buff));
    //io_write(_console.history_fd, buff, strlen(buff));
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

void con_init()
{
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

static void _write_char(uint8_t c, uint8_t col, uint8_t row)
{
    dsp_scroll_char(c, col, row);
}

void con_putc(uint8_t c)
{
    if (c == KEY_BACKSPACE)
    {        
        if (_console.cursor_x > 0)
        {            
            _console.cursor_x--;
            dsp_remove_scroll(_console.cursor_x, _console.cursor_y);
        }
    }
    else if (c == KEY_DEL)
    {
        dsp_remove_scroll(_console.cursor_x, _console.cursor_y);
    }
    else if(c == KEY_TAB)
    {
        _console.cursor_x += 4;
    }
    else if (c == KEY_LEFT)
    {
        if(_console.cursor_x > 0)
            _console.cursor_x--;
    }
    else if (c == KEY_RIGHT)
    {
        if (_console.cursor_x < SCREEN_WIDTH - 1)
            _console.cursor_x++;
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
        dsp_scroll_char(c, _console.cursor_x, _console.cursor_y);
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
