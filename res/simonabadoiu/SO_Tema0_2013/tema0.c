/* Badoiu Simona Andreea 331CB - Tema0 HashTable */
#include "tema0.h"

int main(int argc, char* argv[]) {
	unsigned int dim = atoi(argv[1]);
	int i;
	/* alocam memorie pentru hashtable */
	bucket *hashtable = calloc(dim, sizeof(bucket));
	DIE(hashtable == NULL, "Nu s-a putut face alocarea!");
	/* daca am primit fisiere din care sa citim comenzile, atunci citim din acestea */
	if (argc > 2) {
		for (i = 2; i < argc; i++) {
			citesteFisier(argv[i], &hashtable, &dim);
		}
	}
	/* altfel, vom citi comenzile de la stdin */
	else {
		citesteStdin(&hashtable, &dim);
	}
	/* eliberam memoria alocata pentru hashtable */
	clear(&hashtable, dim);
	free(hashtable);
	return 0;
}







