// Nume Prenume Grupa - Tema 1 Multi-platform Development
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "hash.h"

//#define DEBUG 1

int main(int argc, char** argv)
{
	int hash_length = 0;
	hash_t* hash_table;
	DIE(argc < 2, "Not enough arguments\n");
	
	hash_length = atoi(argv[1]);
	
	hash_table = new_hash(hash_length);
	DIE(hash_table == NULL, "Error in creating new hash table\n");
	
	read_input(argc, argv, hash_table);
	clear_table(hash_table);
	free(hash_table->head);
	free(hash_table);
	
	return 0;
}

