#include <sys/keyboard.h>
#include <sys/key_codes.h>

#include <stdio.h>

const char* kb_key_name(uint8_t code)
{
 //   printf("Name of %d\n", code);

    if (code == KEY_ESC)
        return "ESC";
    if (code == KEY_BACKSPACE)
        return "BACKSPACE";
    if (code == KEY_TAB)
        return "TAB";
    if (code == KEY_NEW_LINE)
        return "NEW LINE";
    if (code == KEY_DEL)
        return "DELETE";
    if (code == KEY_LEFT)
        return "LEFT";
    if (code == KEY_RIGHT)
        return "RIGHT";
    if (code == KEY_UP)
        return "UP";
    if (code == KEY_DOWN)
        return "DOWN";
    if (code == KEY_INSERT)
        return "INSERT";
    if (code == KEY_PG_DOWN)
        return "PG_DOWN";
    if (code == KEY_PG_UP)
        return "PG_UP";
    if (code == KEY_HOME)
        return "HOME";
    if (code == KEY_END)
        return "END";
    if (code == KEY_LSHIFT)
        return "LSHIFT";
    if (code == KEY_RSHIFT)
        return "RSHIFT";
    if (code == KEY_LALT)
        return "LALT";
    if (code == KEY_RALT)
        return "RALT";
    if (code == KEY_LCTRL)
        return "LCTRL";
    if (code == KEY_RCTRL)
        return "RCTRL";
    if (code == KEY_CAPS_LOCK)
        return "CAPS";
    if (code == KEY_F1)
        return "F1";
    if (code == KEY_F2)
        return "F2";
    if (code == KEY_F3)
        return "F3";
    if (code == KEY_F4)
        return "F4";
    if (code == KEY_F5)
        return "F5";
    if (code == KEY_F6)
        return "F6";
    if (code == KEY_F7)
        return "F7";
    if (code == KEY_F8)
        return "F8";
    if (code == KEY_F9)
        return "F9";
    if (code == KEY_F10)
        return "F10";
    if (code == KEY_F11)
        return "F11";
    if (code == KEY_F12)
        return "F12";
    if (code == KEY_VOL_UP)
        return "VOL UP";
    if (code == KEY_VOL_DOWN)
        return "VOL DOWN";
    if (code == KEY_VOL_MUTE)
        return "VOL MUTE";

    static char _buff[2];
    _buff[0] = code;
    _buff[1] = 0;
    return _buff;
}

bool kb_is_shift(kb_state_t* s)
{
    return s->lshift || s->rshift;
}

bool kb_is_alt(kb_state_t* s)
{
    return s->lalt || s->ralt;
}

bool kb_is_ctrl(kb_state_t* s)
{
    return s->lctrl || s->rctrl;
}
