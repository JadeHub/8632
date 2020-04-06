#pragma once

#include <stddef.h>
#include "shell.h"

void ls_cmd(size_t count, char* params[], shell_state_t*);
void cd_cmd(size_t count, char* params[], shell_state_t*);
void clear_cmd(size_t count, char* params[], shell_state_t*);