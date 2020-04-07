#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <sys/key_codes.h>

#include <sys/keyboard.h>

void kb_init();

typedef void (*kb_event_handler)(kb_event_t*);

void kb_register_event_handler(kb_event_handler);
void kb_remove_event_handler();
