#include <stdlib.h>

void entry()
{
	char* msg = "Hello from user land3";

	asm volatile("mov $4, %%eax; mov %0, %%ebx; int $0x64;" :: "b"(msg));

	//msg[5] = 'j';

	//asm volatile("mov $3, %%eax; mov $0, %%ebx; int $0x64;"::);
	exit(7);

	for(;;);
}