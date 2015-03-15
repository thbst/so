#include <stdio.h>

#define DEBUG 1

#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif

int main(void)
{
	int var=1;
	printf("SO ... hello world!\n");
	dprintf("debug message var=%d\n",var);
	return 0;
}
