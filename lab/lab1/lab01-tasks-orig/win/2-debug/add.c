/**
 * SO, 2011
 * Lab #1, Introducere
 *
 * Task #2, Linux
 * 
 * Multiple source files compiling
 */


/**
 * add - adds to integers 
 * @a: first integer
 * @b: second integer
 * 
 * RETURNS: sum @a + @b
 */
#include "add.h"

int add(int a, int b)
{
	Dprintf("Adding %d with %d\n", a, b);
	return a + b;
}
