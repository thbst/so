// Nume Prenume Grupa - Tema 1 Multi-platform Development
#ifndef _UTIL_H
#define _UTIL_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "hash.h"
// chunk size
#define CSIZE 32

/* useful macro for handling error codes */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)


#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif

//structura nod
typedef struct node {
	char* data;
	struct node* next;
} node;

//structura hash table
typedef struct hash_t {
	node** head;
	int length;
} hash_t;

//functie ce aloca memorie pentru un nou hash_table de anumita marime
//data ca parametru
hash_t* new_hash(int hash_size);
//cauta cuvant in hashtable si scrie True/False pe o linie noua in
//fisierul specificat sau la consola daca acest parametru lipseste
node* find_word(hash_t* hash_table, char *search_word);
//adaug cuvant in hash
int add_word(hash_t* hash_table, char* new_word);
//sterge cuvant
int remove_word(hash_t* hash_table, char* data);
//sterge tabel
void clear_table(hash_t* hash_table);
//afiseaza buchet in fisier
void print_bucket(hash_t* hash_table, int key, FILE *f);
//afiseara hash in fisier
void print_hash(hash_t* hash_table, FILE* f);
//redimensioneaza hash la o noua marime
void resize_hash(int new_size, hash_t* hash_table);
//dubleaza marimea hash-ului
void resize_double(hash_t* hash_table);
//injumatateste marimea hash-ului
void resize_halve(hash_t* hash_table);
//ruleaza comenzi
int run_command(hash_t* hash_table, char* line);
//citeste fisier
void read_file(hash_t* hash_table, FILE* f);
//citeste input
void read_input(int argc, char** argv, hash_t* hash_table);
#endif
