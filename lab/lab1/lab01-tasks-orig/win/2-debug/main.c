/**
 * SO, 2011
 * Lab #1, Introducere
 *
 * Task #2, Linux
 * 
 * Multiple source files compiling
 */


#include <stdio.h>

#include "add.h"

#define A_NUMBER	2
#define B_NUMBER	A_NUMBER + 8

int main(void)
{

	add(A_NUMBER, B_NUMBER);
	
	return 0;
}
