#include <stdio.h>
#include "util.h"

#define DEBUG 1
#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif

void functie()
{
	printf("Current line %d in file %s \n", __LINE__, __FILE__ );
	dprintf("test message from util.c\n");
}

/*
int main()
{
	return 0;
}
*/
