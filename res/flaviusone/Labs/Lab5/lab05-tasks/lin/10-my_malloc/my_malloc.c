/**
  * SO, 2014
  * Lab #5
  *
  * Task #10, lin
  *
  * Dump malloc implementation that only allocates space
  */
#include <unistd.h>
#include <sys/types.h>

#include "my_malloc.h"


void * my_malloc(size_t size)
{
	void *current = NULL;
	void *rc = NULL;

	/* TODO - find the current location of the program break */

	/* TODO - user sbrk to alloc at least size bytes */

	/* return the start of the new allocated area */
	return current;
}
