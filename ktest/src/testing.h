#pragma once

#include <stddef.h>
#include <stdbool.h>

void test_assert(bool cond, const char* file, int line);

#define ASSERT_NE(a, b) test_assert((a != b), __FILE__, __LINE__)
#define ASSERT_EQ(a, b) test_assert((a == b), __FILE__, __LINE__)
#define ASSERT_GE(a, b) test_assert(((a) >= (b)), __FILE__, __LINE__)

typedef void (*test_cb_fn_t)(void);

void execute_test(const char* test_suite, const char* test_case, test_cb_fn_t cb);
int test_print_summary();

#define EXEC_TEST(tsuite, tcase) execute_test(tsuite, #tcase, &tcase)