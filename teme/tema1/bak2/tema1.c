// Nume Prenume Grupa - Tema 1 Multi-platform Development
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

void testUnit(int length) {
	char stuff[10][10]= { "Acesta", "este", "un", "test", "folosit", "pentru", "tema", "la", "sisteme", "operare" };
	int key, i;
	//char cmd[20][20] = { "add Acesta", "add este", "add un" };
	//char *cmd = "add test";
	FILE *f1 = fopen("test_bucket", "w");
	FILE *f2 = fopen("test_hash", "w");
	hash_t* tabela = new_hash(length);
	dprintf("length=%d\n", length);
	for (i=0; i<10; i++) {
		key  = hash(stuff[i],length);
		dprintf("word=%s key=%d\n", stuff[i], key);
		add_word(tabela, stuff[i]);
		dprintf("cmd=\"add %s\"\n", stuff[i]);
		//run_command(tabela,cmd);
		//dprintf("cmd=%s\n",cmd);
		//dprintf("\n");
	}
	for (i=0; i<10; i++) {
		print_bucket(tabela,i,f1);
	}
	print_hash(tabela,f2);
}

int main(int argc, char** argv)
{
	int hash_length = 0;
	hash_t* hash_table;
	DIE(argc < 2, "Not enough arguments\n");
	
	hash_length = atoi(argv[1]);
	
	testUnit(hash_length);

	hash_table = new_hash(hash_length);
	DIE(hash_table == NULL, "Error in creating new hash table\n");
	
	read_input(argc, argv, hash_table);
	clear_table(hash_table);
	free(hash_table->head);
	free(hash_table);
	
	return 0;
}

