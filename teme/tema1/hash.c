#include "hash.h"

/**
 * Functie de hash bazata pe djb2 a lui Dan Bernstein
 * http://www.cse.yorku.ca/~oz/hash.html
 * @return valoarea de dispersie (cheia)
 */
unsigned int hash(const char *str, unsigned int hash_length)
{
	unsigned int hash_result = 5381;
	int c;

	while ( (c = *str++) != 0 )
		hash_result = ((hash_result << 5) + hash_result) + c; /* hash * 33 + c */

	return (hash_result % hash_length);
}

