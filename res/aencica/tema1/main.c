/*
	Encica Alexandru
	Grupa 332CC
	Tema 1 SO
*/

#include "utils.h"

int main (int argc, const char *argv[]){
	int size,i;
	HashTable *ht;
	FILE *in_file;

	if (argc == 1){
		perror("Usage: ./tema1 <nr buckets>  [input_files]");
		exit(1);
	}

	size = atoi(argv[1]);
	ht = init(size);

	if (argc == 2){
		in_file = stdin;
		parseFile(ht, in_file);
	}
	else if (argc > 2){
		for (i = 2 ; i < argc ; i++){
			in_file = fopen (argv[i],"r");
			DIE(!in_file,"invalid file!");
			ht = parseFile(ht, in_file);
			fclose(in_file);
		}
	}
	clear(ht);
	free(ht->table);
	free(ht);
	return 0;
}