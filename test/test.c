#include <stdio.h>


int main(int argc, char* argv[])
{
	char str1[1001] = { 0 };
	fgets(str1, 1001, stdin);
	printf("fgets \'%s\'\n", str1);

	return 0;
}