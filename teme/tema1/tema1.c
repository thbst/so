/* Nume Prenume Grupa - Tema 1 Multi-platform Development */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "hash.h"

#define DEBUG 1
#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif


int main(int argc, char *argv[]) {
	HashTable *hash_table = NULL;
	int hash_length, i = 2;
	FILE *f;

	DIE(argc < 2, "Usage: ./tema1 <hash_length> [<file1> .. <filen>]");
	hash_length = atoi(argv[1]);
	fprintf(stderr, "hash_length: %d", hash_length);

	init_hash(&hash_table, hash_length);

	/* Input from stdin */
	if (argc == 2) {
		parse_file(stdin, &hash_table);
	}
	else {
		while ( i<argc ) {
			f = fopen(argv[i], "r");
			DIE(f == NULL, "Could not open file");
			parse_file(f, &hash_table);
			fclose(f);
			i++;
		}
	}

	free_hash(&hash_table);

	return 0;
}
