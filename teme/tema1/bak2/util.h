/* Nume Prenume Grupa - Tema 1 Multi-platform Development */
#ifndef _UTIL_H
#define _UTIL_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "hash.h"
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

/*
#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif
*/

typedef struct node {
	char* data;
	struct node* next;
} node;


typedef struct hash_t {
	node** head;
	int length;
} hash_t;



hash_t* new_hash(int hash_size);


node* find_word(hash_t* hash_table, char *search_word);

int add_word(hash_t* hash_table, char* new_word);

int remove_word(hash_t* hash_table, char* data);

void clear_table(hash_t* hash_table);

void print_bucket(hash_t* hash_table, int key, FILE *f);

void print_hash(hash_t* hash_table, FILE* f);

void resize_hash(int new_size, hash_t* hash_table);

void resize_double(hash_t* hash_table);

void resize_halve(hash_t* hash_table);

int run_command(hash_t* hash_table, char* line);

void read_file(hash_t* hash_table, FILE* f);

void read_input(int argc, char** argv, hash_t* hash_table);
#endif
