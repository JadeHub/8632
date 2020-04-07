#pragma once

#include <stdint.h>

typedef struct shell_state
{
	//uint32_t current_dir_fd;
	char current_dir[256];

}shell_state_t;

typedef struct params
{
	int count;
	const char* params[64];
}params_t;