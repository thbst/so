/* Nume Prenume Grupa - Tema 1 Multi-platform Development */
#ifndef _UTIL_H
#define _UTIL_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#define CMDSIZE 32
#define BUFSIZE 99999

/* macro pentru gestiunea codurilor de eroare */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)

/*
#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif
*/

/* structura unui nod dintr-o lista */
typedef struct Node_ {
	char *data;
	struct Node_ *next;
}Node;

typedef Node *List;

/* structura ce tine perechea key-valoare pentru un hash */
typedef struct Bucket_ {
	List words;
}Bucket;

/* structura pentru un tabel de dispersie */
typedef struct HashTable_ {
	int hash_length;
	Bucket *hash_t;
}HashTable;


/* dezalocarea resurselor si stergerea listei */
void clear_list(List *list);

/* insereaza un element in lista */
int add_member(List *list, char *member);

/* cauta un element in lista */
int find_member(List list, char *member);

/* sterge un element din lista */
int remove_member(List *list, char *member);

/* printeaza lista intr-un fisier */
int print_node(FILE *file, List list);

/* creeaza un tabel de dispersie de o anumita lungime */
void init_hash(HashTable **hash_t, int hash_length);

/* sterge un tabel de dispersie si elibereaza memoria */
void free_hash(HashTable **hash_t);

/* adauga un cuvant intr-un tabel de dispersie */
int add_word(HashTable *hash_t, char *word);

/* sterge un cuvant dintr-o tabela de dispersie */
int remove_word(HashTable *hash_t, char *word);

/* cauta un cuvant intr-o tabela de dispersie */
int find_word(char *word, HashTable hash_t);

/* printeaza un bucket intr-un fisier, fiecare cuvant separat prin spatiu */
int print_bucket(FILE *file,HashTable hash_t, int key);

/* printeaza o tabela de dispersie intr-un fisier, fiecare bucket pe o linie*/
void print_hash(FILE *file, HashTable hash_t);

/* dubleaza marimea unei tabele de dispersie */
int resize_double(HashTable **hash_t);

/* injumatateste marimea unei tabele de dispersie */
int resize_half(HashTable **hash_t);

/* afiseaza cuvant in fisier */
void print_word(FILE *f, int result);

/* ruleaza comenzi */
void run_operation(HashTable **hashTable, char buf[3][BUFSIZE]);

/* proceseaza fisiere de intrare/iesire */
void parse_file(FILE *file, HashTable **hashTable);


#endif
