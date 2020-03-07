#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>

void test_io()
{
	uint32_t fd = sys_open("/initrd/temp/f1.txt", 0);

	if (fd == 0xffffffff)
	{
		printf("Failed to open file");
	}
	else
	{
		printf("Opened file ");

		size_t l = 80;
		char buff[80];
		memset(buff, 0, l);
		sys_read(fd, buff, l-1);
		printf(buff);
		sys_close(fd);
	}
}

void entry()
{
	char* msg = "Hello from user land %d";

	printf("printf %d %s\n", 2, "testing");

	test_io();
	
	sys_exit(5);

	for(;;);
}