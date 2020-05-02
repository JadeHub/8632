#include <stdlib.h>

int abs(int n)
{
	return n < 0 ? n * -1 : n;
}

long labs(long n)
{
	return n < 0 ? n * -1 : n;
}

long long llabs(long long n)
{
	return n < 0 ? n * -1 : n;
}