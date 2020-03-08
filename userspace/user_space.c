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

void test_sleep()
{
	printf("Sleeping\n");

	sys_sleep_ms(1000);

	printf("Sleep finished\n");
}

void test_kb()
{
	uint32_t fd = sys_open("/dev/keyboard", 0);
	if (fd == 0xffffffff)
	{
		printf("Failed to open keyboard");
	}
	else
	{
		char buff;
		
		do
		{
			sys_read(fd, &buff, 1);

			printf("%c", buff);
		} while (buff != 'q');
	}
}

void entry()
{
	char* msg = "Hello from user land %d";

	printf("printf %d %s\n", 2, "testing");

	//test_io();

	//for(int j=0;j<5;j++)
		//test_sleep();
	
	test_kb();

	sys_exit(4);

	printf("exit returned\n");
	for(;;);
}