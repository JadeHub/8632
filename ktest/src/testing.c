#include "testing.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define RED_TEXT "\033[91;40m"
#define GREEN_TEXT "\033[92;40m"
#define RESET_TEXT "\033[0m"

static const char* _cur_test = NULL;
static uint32_t _failure_count = 0;
static uint32_t _test_count = 0;
static bool _test_success = true;

bool test_assert(bool cond, const char* file, int line)
{
	if (!cond)
	{
		printf("[ %sFAILURE%s  ] %s at %s:%d\n", RED_TEXT, RESET_TEXT, _cur_test, file, line);
		
		_test_success = false;
	}
	return cond;
}

void execute_test(const char* test_suite, const char* test_case, test_cb_fn_t cb)
{
	_cur_test = test_case;
	_test_success = true;

	printf("[ RUN      ] %s.%s\n", test_suite, test_case);

	cb();
	_test_count++;
	if (_test_success)
	{
		printf("[ %sSUCCESS%s  ] %s.%s\n", GREEN_TEXT, RESET_TEXT, test_suite, test_case);
	}
	else
	{
		_failure_count++;
		printf("[ %sFALIURE%s  ] %s.%s\n", RED_TEXT, RESET_TEXT, test_suite, test_case);
	}
	printf("[ -------- ]\n");
}

int test_print_summary()
{
	
	if (_failure_count > 0)
	{
		printf("[ %sCOMPLETE%s ] %d / %d failed\n", RED_TEXT, RESET_TEXT, _failure_count, _test_count);
		return -1;
	}
	printf("[ %sCOMPLETE%s ] %d tests passed\n", GREEN_TEXT, RESET_TEXT, _test_count);
	return 0;
}