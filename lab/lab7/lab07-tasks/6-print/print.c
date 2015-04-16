#include <stdio.h>

int main(void)
{
	fprintf(stdout, "A\n");
	fprintf(stderr, "B");
	sleep(5);
	return 0;
}
