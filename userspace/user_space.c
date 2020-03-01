#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

void print(const char* str)
{
	SYSCALL1(SYSCALL_PRINT, str);
}

uint32_t open(const char* path, uint32_t flags)
{
	return (uint32_t)SYSCALL2(SYSCALL_OPEN, path, flags);
}

size_t read(uint32_t fd, char* buff, size_t sz)
{
	return (size_t)SYSCALL3(SYSCALL_READ, fd, buff, sz);
}

void close(uint32_t fd)
{
	SYSCALL1(SYSCALL_CLOSE, fd);
}


void test_io()
{
	uint32_t fd = open("/initrd/temp/f1.txt", 0);

	if (fd == 0xffffffff)
	{
		print("Failed to open file");
	}
	else
	{
		print("Opened file");

		size_t l = 80;
		char buff[80];
		memset(buff, 0, l);
		read(fd, buff, l-1);
		print(buff);
		//print("\n");
	}
}

void entry()
{
	char* msg = "Hello from user land %d";

	printf("printf %d %s\n", 2, "testing");
	int x = testj();
	//asm volatile("mov $4, %%eax; mov %0, %%ebx; int $0x64;" :: "b"(msg));

	test_io();
	//msg[5] = 'j';

	//asm volatile("mov $3, %%eax; mov $0, %%ebx; int $0x64;"::);
	exit(5);

	for(;;);
}