/**
 * SO, 2014 - Lab #07, Profiling
 * Task #2, Linux
 *
 * Column major
 */
#include <stdio.h>

#define SIZE 4096

char x[SIZE][SIZE];

int
main(void)
{
	int i, j;

	for (i = 0; i < SIZE; ++i) {
		for (j = 0; j < SIZE; ++j) {
			x[j][i]++;
		}
	}
	return 0;
}
