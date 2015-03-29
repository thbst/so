/* Tudorica Constantin-Alexandru, 333CA */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define LINE_SIZE 20002
#define DELIMITERS " \n\t"
#define DEBUG 0

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)



unsigned int bucketsSize;


typedef struct InternalElement {
	struct InternalElement * next;
	char* data;
} Element;

Element** buckets;

/* Aparent functia strdup nu e C89 asa ca a trebuit sa imi implementez
 * propria functie.
 */
static char * strdup2(char *str) {
	char *r;
	unsigned int l;
	if (str == NULL)
		return str;
	l = strlen(str)+1;
	r = calloc(l, sizeof(char));
	DIE(r == NULL, "Nu am putut aloca memorie.");
	memcpy(r, str, sizeof(char)*l);
	return r;
}

void addWord(char* word) {
	unsigned int key;
	Element **x;
	if (DEBUG) fprintf(stderr," %s\n", word);
	if (word == NULL)
		return;
	key = hash(word, bucketsSize);
	x = &(buckets[key]);

	/* Folosesc double star ideea[0], adaptata la functia de adaugare, pentru
	 * a nu trata cheia in mod special.
	 * http://wordaligned.org/articles/two-star-programming
	 */
	while (*x!=NULL && strcmp((*x)->data, word)!=0)
		x = &((*x)->next);

	if (*x!=NULL) /* Inseamna ca mai avem cuvantul deja */
		return;

	*x = calloc(1, sizeof(Element));
	DIE(x == NULL, "Nu am mai putut aloca memorie.");
	(*x)->data = strdup2(word);
	(*x)->next = NULL;
}

void removeWord(char* word) {
	unsigned int key;
	Element **x;
	Element *temp;
	if (DEBUG) fprintf(stderr, " %s\n", word);
	if (word == NULL)
		return;
	key = hash(word, bucketsSize);
	x = &buckets[key];

	while (*x!=NULL && strcmp((*x)->data, word)!=0)
			x = &((*x)->next);

	if (*x==NULL) /* Inseamna ca mai avem cuvantul deja */
		return;

	temp = *x;
	*x = (*x)->next;
	free(temp->data);
	free(temp);
}

void findWord(char *filename, char * word) {
	unsigned int key;
	Element **x; /*TODO(tudalex): Name it to smth else*/
	FILE *f;
	if (DEBUG) fprintf(stderr, " %s\n", word);
	if (word == NULL)
		return;

	if (filename != NULL) {
		f = fopen(filename, "a");
		DIE(f == NULL, "Nu am putu deschide fisierul de iesire.");
	} else
		f = stdout;

	key = hash(word, bucketsSize);
	x = &buckets[key];

	while (*x!=NULL && strcmp((*x)->data, word)!=0)
			x = &((*x)->next);

	if (*x != NULL)
		fprintf(f, "True\n");
	else
		fprintf(f, "False\n");

	if (filename != NULL)
		DIE(fclose(f) != 0, "Nu am putut inchide fisierul");

}

/* Functia care afiseaz la output un anumit bucket.*/
void printBucket2(char* filename, int key) {
	Element **x;
	FILE *f;

	x = &buckets[key];
	if (filename != NULL) {
		f = fopen(filename, "a");
		DIE(f == NULL, "Nu am putut deschide fisierul de iesire.");
	}
	 else
		f = stdout;

	while (*x!=NULL) {
		fprintf(f,"%s ", (*x)->data);
		x = &((*x)->next);
	}

	fprintf(f, "\n");
	if (filename != NULL)
		DIE(fclose(f)!=0, "Nu am putut inchide fisierul de iesire.");
}

/* Wrapper pentru functia printBucket2*/
void printBucket(char * filename, char * bucket) {
	int key = atoi(bucket);
	if (DEBUG) fprintf(stderr, "%s %s\n", bucket, filename);
	printBucket2(filename, key);
}

/* Functia care implementeaza comanda print.
 * Am creat functia printBucket2 special pentru a o putea apela din aceasta.
 */
void print(char * filename) {
	unsigned int i;
	for (i = 0; i < bucketsSize; ++i)
		if (buckets[i] != NULL)
			printBucket2(filename, i);
}
/* Functia care face resize */
void resize2(int oldHashSize, int newHashSize) {
	Element **oldBuckets = buckets;
	Element *x;
	int i;
	buckets = calloc(newHashSize, sizeof(Element*));
	DIE(buckets == NULL, "Nu am putut aloca memorie.");
	for (i = 0; i < oldHashSize; ++i) {
		x = oldBuckets[i];
		while (x!=NULL) {
			Element *t = x;
			Element **xt;
			int key = hash(t->data, newHashSize);
			x = x->next;
			t->next = NULL;
			xt = &buckets[key];
			while (*xt != NULL)
				xt = &((*xt)->next);
			*xt = t;
		}
	}
	free(oldBuckets);
}

/* Wrapper pentru functia care face resize.*/
void resize(char * mode) {
	if (strcmp(mode, "double") == 0) {
		resize2(bucketsSize, bucketsSize*2);
		bucketsSize*=2;
	} else if (strcmp(mode, "halve") == 0) {
		resize2(bucketsSize, bucketsSize/2);
		bucketsSize/=2;
	}
}

void clear(void) {
	unsigned int i;
	for (i = 0; i < bucketsSize; ++i)
	if (buckets[i] != NULL) {
		Element *x = buckets[i];
		while (x!=NULL) {
			Element *t = x;
			x = x->next;
			free(t->data);
			free(t);
		}
		buckets[i] = NULL;
	}
}

void processCommands(FILE *f) {
	char buffer[LINE_SIZE];
	char* command;
	while (fgets(buffer, LINE_SIZE, f)) {

		command = strtok(buffer, DELIMITERS);
		if (DEBUG) fprintf(stderr,"%s", command);
		if (command == NULL) /* Linia e goala*/
			continue;

		if (strcmp(command, "add") == 0) {
			addWord(strtok(NULL, DELIMITERS));
		} else if (strcmp(command, "remove") == 0) {
			removeWord(strtok(NULL, DELIMITERS));
		} else if (strcmp(command, "find") == 0) {
			findWord(strtok(NULL, DELIMITERS), strtok(NULL, DELIMITERS));
		} else if (strcmp(command, "print_bucket") == 0) {
			printBucket(strtok(NULL, DELIMITERS), strtok(NULL, DELIMITERS));
		} else if (strcmp(command, "print") == 0) {
			print(strtok(NULL, DELIMITERS));
		} else if (strcmp(command, "resize") == 0) {
			resize(strtok(NULL, DELIMITERS));
		} else if (strcmp(command, "clear") == 0) {
			clear();
		}
	}
}

int main(int argc, char *argv[]) {
	int i;
	if (argc == 1)
		return 0;
	bucketsSize = atoi(argv[1]);
	buckets = calloc(bucketsSize, sizeof(Element*));
	DIE(buckets == NULL, "Nu am putut aloca memorie.");

	if (argc >2)
		for (i = 2; i < argc; ++i) {
			FILE *tf = fopen(argv[i], "r");
			DIE(tf == NULL, "Nu am putut deschide fisierul de intrare");
			processCommands(tf);
			DIE(fclose(tf), "Nu am putut inchide fisierul de intrare.");
		}
	else
		processCommands(stdin);
	clear();
	free(buckets);
	return 0;
}
