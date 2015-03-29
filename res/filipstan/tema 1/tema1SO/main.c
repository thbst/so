// Stan Filip Ioan 332 CA 

#include "hashtable.h"

#define BUFSIZE 20000

struct hashtable* execute(char* buf, struct hashtable *H){
	char *p;
	int exist;
	unsigned int index_bucket;
	FILE* out;

	p = strtok(buf, " \n");
	// add "word"
	if (strcmp(p, "add") == 0){
		p = strtok(NULL, " \n");
		put(H, p);
	}// remove "word"
	else if (strcmp(p, "remove") == 0){
		p = strtok(NULL, " \n");
		removeWord(H, p);
	}// clear
	else if (strcmp(p, "clear") == 0){
		clearHashtable(H);
	}// find "word" [file.out]
	else if (strcmp(p, "find") == 0){
		p = strtok(NULL, " \n");
		exist = find(H, p);

		p = strtok(NULL, " \n");
		if (p != NULL){
			out = fopen(p, "a");
			DIE(out == NULL, "open out file error [find]");
		} else {
			out = stdout;
		}

		if (exist == 1){
			fprintf(out, "True\n\n");
		} else {
			fprintf(out, "False\n\n");
		}

		if (out != stdout){
			DIE(fclose(out) != 0, "eroare la close [find]");
		}
	}// print_bucket index_bucket [file.out]
	else if (strcmp(p, "print_bucket") == 0){
		p = strtok(NULL, " \n");
		index_bucket = (unsigned int)atoi(p);

		p = strtok(NULL, " \n");
		if (p != NULL){
			out = fopen(p, "a");
			DIE(out == NULL, "open out file error [print_bucket]");
		} else {
			out = stdout;
		}
					
		printList(out, &H->table[index_bucket]);
		fprintf(out, "\n");
					
		if (out != stdout){
			DIE(fclose(out) != 0, "eroare la close [print_bucket]");
		}
	}// print [file.out]
	else if (strcmp(p, "print") == 0){
		p = strtok(NULL, " \n");
		if (p != NULL){
			out = fopen(p, "a");
			DIE(out == NULL, "open out file error [print]");
		} else {
			out = stdout;
		}
					
		printHashtable(out, H);

		if (out != stdout){
			DIE(fclose(out) != 0, "eroare la close [print]");
		}
	}
	else if (strcmp(p, "resize") == 0){
		p = strtok(NULL, " \n");
		if (strcmp(p, "double") == 0){
			H = resizeHashtable(H, 2);
		} else {
			H = resizeHashtable(H, 0.5);
		}
	}
	return H;
}

struct hashtable* readCommands(FILE* in, char* buf, struct hashtable *H){
	while (fgets(buf, BUFSIZE, in) != NULL){
			if (strcmp(buf, "\n") == 0){
				continue;
			}
			H = execute(buf, H);
	}
	return H;
}

int main(int argc, char *argv[]){

	unsigned int hash_size;
	int i;
	struct hashtable *H;
	char buf[BUFSIZE];
	FILE *f;

	if (argc < 2){
		fprintf(stderr, "Usage ./tema1 HASH_SIZE [file1.in] [file2.in] ...\n");
		return 1;
	}
	
	hash_size = atoi(argv[1]);
	H = (struct hashtable *)malloc(sizeof(struct hashtable));
	initHashtable(H, hash_size);

	if (argc == 2){
		H = readCommands(stdin, buf, H);
	} else {
		for (i = 2; i < argc; i++){
			f = fopen(argv[i], "r");
			DIE(f == NULL, "open input file fails\n");
			H = readCommands(f, buf, H);
			DIE(fclose(f) != 0, "error on close file");
		}
	}
	destroyHashtable(H);
	free(H);
	
	return 0;
}