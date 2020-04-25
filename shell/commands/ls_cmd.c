#include "shell.h"
#include "built_in.h"

#include <dirent.h>
#include <stdio.h>

void ls_cmd(size_t count, const char* params[], shell_state_t* shell)
{
	const char* path = count == 1 ? shell->current_dir : params[1];
	struct DIR* d = opendir(path);

	if (!d)
	{
		printf("Can't read directory %s\n", path);
		return;
	}
	//printf("DIR Oopen\n");
	struct dirent de;
	while (readdir(d, &de))
	{
		//if (!de)
			//break;
		printf("%s\n", de.name);
	}
	closedir(d);
}