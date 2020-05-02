#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "testing.h"

extern void exec_malloc_tests();

int main(int argc, char* argv[])
{
	exec_malloc_tests();

	return test_print_summary();
}