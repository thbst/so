#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

typedef struct bucket { 
	int size;
	int curent;
	char** cuvinte;
}bucket;

typedef struct {
	struct bucket** buckets;
} hashTable;


// adauga un cuvant in tabela
void add_cuvant(hashTable *table,char* cuvant,unsigned int dim_hash);

//sterge un cuvant din tabela
void remove_cuvant(hashTable *table,char* cuvant,unsigned int dim_hash);

//goleste tabela
int clear_hash(hashTable *table,unsigned int dim_hash);

//gaseste un cuvant iar daca exista scrie in fisier 
// sau la stdin true/false
void find_cuvant(hashTable *table,char* cuvant,unsigned int dim_hash,char* numefis);

//scrie cuvintele din bucketul cu acel index in fisier sau la stdin
void print_bucket(hashTable *table,unsigned int index_bucket, char* numefis);

//afiseaza toate bucketurile in fisier sau la stdin
void print_hash(hashTable *table,unsigned int dim_hash,char* numefis);

//redimensioneaza o tabela
void resize_any(hashTable *table, unsigned int *dim_hash,unsigned int new_dim,hashTable * new_table);

//dubleaza dimensiunea hash-ului
void resize_double(hashTable *table,unsigned int *dim_hash);

//injumatateste dimensiunea tabelei
void resize_halve(hashTable *table,unsigned int *dim_hash);

#endif
