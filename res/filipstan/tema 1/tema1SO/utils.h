// Stan Filip Ioan 332 CA 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)

struct node{
	char* word;
	struct node *next;
};