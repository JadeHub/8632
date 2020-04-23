#include <dirent.h>
#include <sys/syscall.h>

#include <stddef.h>

struct DIR* opendir(const char* path)
{
	return sys_opendir(path);
}

void closedir(struct DIR* dir)
{
	sys_closedir(dir);
}

bool readdir(struct DIR* dir, struct dirent* ent)
{
	return sys_readdir(dir, ent);
}