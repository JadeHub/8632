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

struct dirent* readdir(struct DIR* dir)
{
	return sys_readdir(dir);
}