#include "testing.h"

#include <stdlib.h>



void alloc_free()
{
	size_t sz = 20;
	char* ptr = (char*)malloc(sz);
	ASSERT_NE(NULL, ptr);
	for (size_t i = 0; i < sz; i++)
		ptr[i] = 123;
	free(ptr);
}

void not_overlapping()
{
	char* ptrs[10];
	size_t sz = 100;

	for (int i = 0; i < 10; i++)
		ptrs[i] = (char*)malloc(sz);

	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
		{
			if (i == j)continue;
			ASSERT_NE(ptrs[i], ptrs[j]); //cant be equal
			ASSERT_GE(abs(ptrs[i] - ptrs[j]), sz); //must be more than sz bytes appart
		}

	for (int i = 0; i < 10; i++)
		free(ptrs[i]);
}

void exec_malloc_tests()
{
	EXEC_TEST("malloc", alloc_free);
	EXEC_TEST("malloc", not_overlapping);
}