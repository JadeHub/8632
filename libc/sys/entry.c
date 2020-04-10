#include <stdlib.h>

extern int main(int, char* []);

void __entry()
{
	exit(main(0, 0));
}