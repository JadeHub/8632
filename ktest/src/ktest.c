#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "testing.h"

extern void exec_malloc_tests();
extern void exec_fatfs_tests();
extern void exec_libc_tests();

int main(int argc, char* argv[])
{
	//return 0;
	exec_libc_tests();
	exec_fatfs_tests();
	exec_malloc_tests();

	return test_print_summary();
}