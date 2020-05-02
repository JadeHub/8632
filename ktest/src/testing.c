#include "testing.h"

#include <stdint.h>
#include <stdio.h>

#define RED_TEXT "\033[91;40m"
#define GREEN_TEXT "\033[92;40m"
#define RESET_TEXT "\033[0m"

static const char* _cur_test = NULL;
static uint32_t _failure_count = 0;
static bool _test_success = true;

void test_assert(bool cond, const char* file, int line)
{
	if (!cond)
	{
		printf("[ %sFAILURE%s  ] %s at %s:%d\n", RED_TEXT, RESET_TEXT, _cur_test, file, line);
		_failure_count++;
		_test_success = false;
	}
}

void execute_test(const char* test_suite, const char* test_case, test_cb_fn_t cb)
{
	//printf("\033[91;40mHello");


	_cur_test = test_case;
	_test_success = true;

	printf("[ RUN      ] %s.%s\n", test_suite, test_case);

	cb();
	if(_test_success)
		printf("[ %SUCCESS%s ] %s.%s\n", GREEN_TEXT, RESET_TEXT, test_suite, test_case);
	else
		printf("[ %FALIURE%s ] %s.%s\n", RED_TEXT, RESET_TEXT, test_suite, test_case);
}

int test_print_summary()
{
	if (_failure_count > 0)
	{
		return -1;
	}
	return 0;
}