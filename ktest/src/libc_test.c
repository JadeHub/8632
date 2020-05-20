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

void strcmp_()
{
	char* s1 = "12345678901";
	char* s2 = "12345678901";

	ASSERT_EQ(0, strcmp(s1, s2));
}

void strncmp_()
{
	char* s1 = "12345678901  ";
	char* s2 = "12345678901xyz";
	ASSERT_EQ(0, strncmp(s1, s2, 11));

	char* s3 = "12345678901";
	char* s4 = "12345678901";
	ASSERT_EQ(0, strncmp(s3, s4, 11));

	char* s5 = "12345";
	char* s6 = "12345678901";
	ASSERT_NE(0, strncmp(s5, s6, 11));

	char* s7 = "12345678902";
	char* s8 = "12345678901";
	ASSERT_NE(0, strncmp(s7, s8, 11));

	char* s9 = "";
	char* s10 = "12345678901";
	ASSERT_NE(0, strncmp(s7, s8, 11));
}

void exec_libc_tests()
{
	EXEC_TEST("libc", strrchr_);
	EXEC_TEST("libc", strncmp_);
	EXEC_TEST("libc", strcmp_);
}