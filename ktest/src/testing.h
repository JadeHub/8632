#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

bool test_assert(bool cond, const char* file, int line);

#define ASSERT_TRUE(c) if(!test_assert((c), __FILE__, __LINE__)) return
#define ASSERT_FALSE(c) if(!test_assert((!c), __FILE__, __LINE__)) return
#define ASSERT_NE(a, b) if(!test_assert((a != b), __FILE__, __LINE__)) return
#define ASSERT_EQ(a, b) if(!test_assert((a == b), __FILE__, __LINE__)) return
#define ASSERT_GE(a, b) if(!test_assert(((a) >= (b)), __FILE__, __LINE__)) return
#define ASSERT_STR_EQ(a, b) if(!test_assert( (strcmp((a), (b)) == 0), __FILE__, __LINE__)) return

#define EXPECT_TRUE(c) test_assert((c), __FILE__, __LINE__)
#define EXPECT_NE(a, b) test_assert((a != b), __FILE__, __LINE__)
#define EXPECT_EQ(a, b) test_assert((a == b), __FILE__, __LINE__)
#define EXPECT_GE(a, b) test_assert(((a) >= (b)), __FILE__, __LINE__)
#define EXPECT_STR_EQ(a, b) test_assert( (strcmp((a), (b)) == 0), __FILE__, __LINE__)


typedef void (*test_cb_fn_t)(void);

void execute_test(const char* test_suite, const char* test_case, test_cb_fn_t cb);
int test_print_summary();

#define EXEC_TEST(tsuite, tcase) execute_test(tsuite, #tcase, &tcase)