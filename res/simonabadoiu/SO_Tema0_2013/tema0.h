/* Badoiu Simona Andreea 331CB - Tema0 HashTable */
#ifndef TEMA_H_
#define TEMA_H_

#include "operatii.h"
#include "executaComenzi.h"

/* useful macro for handling error codes */
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#endif
