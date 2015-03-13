/**
 * SO, 2011
 * Lab #1, Introducere
 *
 * Task #4, Linux
 *
 * Building libraries
 */

#include <stdio.h>
#include "hexdump.h"

int main(void)
{	
	struct xframe frame;
	char buf[64];
	
	generate_frame(&frame);
	
	hex_dump_to_buffer(frame.data, frame.len, 16, 1, buf, 64, 1);
	
	printf("%s \n", buf);
	return 0;
}	
	
