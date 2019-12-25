#include <string.h>

int strcmp(const char* lhs, const char* rhs)
{
	while (lhs && *lhs != 0 && rhs && *rhs != 0)
	{
		if (*lhs != *rhs)
			return *lhs - *rhs;
		lhs++;
		rhs++;
	}
	return 0;
}