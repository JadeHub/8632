#pragma once

#include <sys/cdefs.h>

__LIBC_BEGIN_H

#include <stdint.h>
#include <stdbool.h>

typedef struct kb_state
{
    bool lshift : 1;
    bool rshift : 1;
    bool lalt : 1;
    bool ralt : 1;
    bool lctrl : 1;
    bool rctrl : 1;
    bool caps : 1;
} kb_state_t;

typedef struct kb_event
{
    uint8_t code;
    kb_state_t state;
    bool pressed;
}kb_event_t;

const char* kb_key_name(uint8_t code);
bool kb_is_shift(kb_state_t*);
bool kb_is_alt(kb_state_t*);
bool kb_is_ctrl(kb_state_t*);

__LIBC_END_H