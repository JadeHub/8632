#include "testing.h"

#include <string.h>
#include <stdio.h>

void strrchr_()
{
	char* path = "/fatfs/test";
	char* last_slash = strrchr(path, '/');

	ASSERT_NE(NULL, last_slash);
	ASSERT_STR_EQ(last_slash, "/test");
}

void exec_libc_tests()
{
	EXEC_TEST("libc", strrchr_);
}