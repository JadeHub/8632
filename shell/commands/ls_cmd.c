#include "shell.h"
#include "built_in.h"

#include <dirent.h>
#include <stdio.h>

void ls_cmd(size_t count, char* params[], shell_state_t* shell)
{
	char* path = count == 1 ? shell->current_dir : params[1];
	struct DIR* d = opendir(path);

	if (!d)
	{
		printf("Can't read directory %s\n", path);
		return;
	}
	struct dirent* de;
	while (de = readdir(d))
	{
		//if (!de)
			//break;
		printf("%s\n", de->name);
	}
	closedir(d);
}