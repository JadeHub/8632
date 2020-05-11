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

	//Doesn't exist
	ASSERT_FALSE(_get_dirent(_dir, file, &dent));

	//Create file
	uint32_t fd = sys_open(path, OPEN_CREATE | OPEN_WRITE);
	ASSERT_NE(INVALID_FD, fd);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(0, dent.size);

	//Write some bytes
	char* buff = "Testing 123";
	size_t blen = strlen(buff);
	size_t sz = sys_write(fd, buff, blen);
	ASSERT_EQ(blen, sz);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(sz, dent.size);

	//Close file
	sys_close(fd);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(sz, dent.size);

	//Open file
	fd = sys_open(path, OPEN_READ);
	ASSERT_NE(INVALID_FD, fd);
	char read_buff[64];
	sz = sys_read(fd, read_buff, 64);
	ASSERT_EQ(blen, sz);
	ASSERT_EQ(0, memcmp(read_buff, buff, blen));
	sys_close(fd);

	//Delete file
	ASSERT_EQ(0, sys_remove(path));
	ASSERT_FALSE(_get_dirent(_dir, file, &dent));
}

void big_file()
{
	struct dirent dent;
	char file[256];
	char path[1024];
	_make_tmp_fn(_dir, file);
	sprintf(path, "%s/%s", _dir, file);

	//Doesn't exist
	ASSERT_FALSE(_get_dirent(_dir, file, &dent));

	//Create file
	uint32_t fd = sys_open(path, OPEN_CREATE | OPEN_WRITE);
	ASSERT_NE(INVALID_FD, fd);

	//Write 5120 bytes (10 sectors)
	uint8_t buff[512];
	size_t sz = 0;
	for (size_t sec = 0; sec < 10; sec++)
	{
		memset(buff, sec, 512);
		sz += sys_write(fd, buff, 512);
	}
	ASSERT_EQ(512*10, sz);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(512*10, dent.size);
	sys_close(fd);

	//Read file
	fd = sys_open(path, OPEN_READ);
	ASSERT_NE(INVALID_FD, fd);
	uint8_t* read_buff = (uint8_t*)malloc(512 * 10);
	sz = sys_read(fd, read_buff, 512 * 10);
	ASSERT_EQ(512 * 10, sz);
	sys_close(fd);
	sys_remove(path);

	for (size_t i = 0; i < 512 * 10; i++)
	{
		ASSERT_EQ(i / 512, read_buff[i]);
	}
	free(read_buff);
}

void sector_unaligned()
{
	struct dirent dent;
	char file[256];
	char path[1024];
	_make_tmp_fn(_dir, file);
	sprintf(path, "%s/%s", _dir, file);

	//Doesn't exist
	ASSERT_FALSE(_get_dirent(_dir, file, &dent));

	//Create file
	uint32_t fd = sys_open(path, OPEN_CREATE | OPEN_WRITE);
	ASSERT_NE(INVALID_FD, fd);

	//Write 612 bytes (1 sector + 100 bytes)
	uint8_t buff[612];
	size_t sz = 0;
	for (size_t i = 0; i < 612; i++)
		buff[i] = i % 512;
	sz = sys_write(fd, buff, 612);
	ASSERT_EQ(612, sz);
	ASSERT_TRUE(_get_dirent(_dir, file, &dent));
	ASSERT_EQ(612, dent.size);
	sys_close(fd);

	//Read file
	fd = sys_open(path, OPEN_READ);
	ASSERT_NE(INVALID_FD, fd);
	uint8_t read_buff[612];
	sz = sys_read(fd, read_buff, 612);
	ASSERT_EQ(612, sz);
	ASSERT_EQ(0, memcmp(buff, read_buff, 612));

	//Read two sub-sectors
	sys_fseek(fd, 256, 1);
	sys_read(fd, read_buff, 356);
	ASSERT_EQ(0, memcmp(buff+256, read_buff, 356));

	sys_close(fd);
	sys_remove(path);	
}

void lfn()
{
	char path[1024];
	sprintf(path, "%s/%s", _dir, "longfilename.txt");

	sys_remove(path);

	struct dirent dent;
	//Doesn't exist
	ASSERT_FALSE(_get_dirent(_dir, "longfilename.txt", &dent));

	uint32_t fd = sys_open(path, OPEN_CREATE | OPEN_WRITE);
	ASSERT_NE(INVALID_FD, fd);
	size_t sz = sys_write(fd, "test", 4);
	ASSERT_EQ(4, sz);
	sys_close(fd);

	fd = sys_open(path, OPEN_READ);
	char read_buff[5];
	sz = sys_read(fd, read_buff, 4);
	ASSERT_EQ(4, sz);
	read_buff[4] = '\0';
	ASSERT_EQ(0, strcmp(read_buff, "test"));
	sys_close(fd);
	sys_remove(path);

	//Doesn't exist
	ASSERT_FALSE(_get_dirent(_dir, "longfilename.txt", &dent));
}

void dir()
{
	struct dirent dent;
	char path[1024];
	sprintf(path, "%s/%s", _dir, "tdir");

	sys_remove(path);
	ASSERT_FALSE(_get_dirent(_dir, "tdir", &dent));

	sys_mkdir(path);
	ASSERT_TRUE(_get_dirent(_dir, "tdir", &dent));

	sys_remove(path);
	ASSERT_FALSE(_get_dirent(_dir, "tdir", &dent));
}

void exec_fatfs_tests()
{
	EXEC_TEST("fatfs", create_file);
	EXEC_TEST("fatfs", big_file);
	EXEC_TEST("fatfs", sector_unaligned);
	EXEC_TEST("fatfs", lfn);
	EXEC_TEST("fatfs", dir);
}