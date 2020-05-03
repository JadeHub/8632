#include "testing.h"

#include <sys/io_defs.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>

static const char* _dir = "/fatfs/tmp";

//todo lfns
//overwrite / append

static bool _get_dirent(const char* dir, const char* fn, struct dirent* dirent)
{
	struct DIR* d = opendir(dir);

	if (d)
	{
		while (readdir(d, dirent))
		{
			if (strcmp(fn, dirent->name) == 0)
			{
				closedir(d);
				return true;
			}
		}
	}
	
	return false;
}

static void _make_tmp_fn(const char* dir, char* name)
{
	char buffer[256];
	uint32_t fd;

	for (int i = 0; i < 9999; i++)
	{
		sprintf(buffer, "%s/test-%d.tmp", dir, i);

		fd = sys_open(buffer, OPEN_READ);
		if (fd == INVALID_FD)
		{
			sprintf(name, "test-%d.tmp", i);
			return;
		}
		sys_close(fd);
	}
	ASSERT_TRUE(false);
}


void create_file()
{
	struct dirent dent;
	char file[256];
	char path[1024];
	_make_tmp_fn(_dir, file);
	sprintf(path, "%s/%s", _dir, file);

	printf("Test file %s\n", path);

	//Doesn't exist
	ASSERT_FALSE(_get_dirent(_dir, file, &dent));

	//Create file
	uint32_t fd = sys_open(path, OPEN_CREATE | OPEN_WRITE);
	ASSERT_NE(INVALID_FD, fd);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(0, dent.size);

	//Write some bytes
	char* buff = "Testing 123";
	size_t len = strlen(buff);
	size_t sz = sys_write(fd, buff, len);
	ASSERT_EQ(len, sz);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(sz, dent.size);

	//Close file
	sys_close(fd);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(sz, dent.size);

	//Delete file
	ASSERT_EQ(0, sys_remove(path));
	ASSERT_FALSE(_get_dirent(_dir, file, &dent));
}


void exec_fatfs_tests()
{
	EXEC_TEST("fatfs", create_file);
}