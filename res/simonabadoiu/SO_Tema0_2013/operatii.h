/* Badoiu Simona Andreea 331CB - Tema0 HashTable */
#ifndef OPERATII_H_
#define OPERATII_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"

/* useful macro for handling error codes */
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)
	
// in aceasta structura vom pastra bucketul si numarul de elemente din bucket
typedef struct {
	int n;
	char** elemente;
} bucket;

/* adauga cuvant in hashtable, pe pozitia data de functia hash */
void add(char* cuvant, bucket **hashtable, unsigned int dim);
/* sterge cuvantul cuvant din hashtable */
void rmv(char* cuvant, bucket **hashtable, unsigned int dim);
/* goleste toate bucketurile din hashtable */
void clear(bucket **hashtable, unsigned int dim);
/* cauta un cuvant in hashtable si scrie true/false in fis/stdin in functie de rezultat */
void find(char* cuvant, bucket *hashtable, unsigned int dim, char* fisier);
/* afiseaza toate elemntele bucket-ului de la index_bucket.
Stim ca index_bucket este intotdeauna valid */
void print_bucket(unsigned int index_bucket,
				  bucket *hashtable, 
				  unsigned int dim,
				  char* fisier);
/* afiseaza toate bucketurile din hashtable, fiecare pe o linie.
Afisarea se face in fisier, daca parametru fisier nu este NULL, sau
la stdin in caz contrar */
void print_all(bucket *hashtable, unsigned int dim, char* fisier);
/* redimensioneaza hashtable-ul si adauga cuvintele pe pozitiile
corespunzatoare in noul bucket */
bucket* resize(bucket **hashtable, unsigned int dim, unsigned int dim_noua);
/* foloseste resize pentru a obtine un bucket cu dimensiunea dubla fata
de dimensiune actuala */
bucket* resize_double(bucket **hashtable, unsigned int *dim);
/* foloseste resize pentru a obtine un bucket cu dimensiunea dim/2 fata
de dimensiune actuala */
bucket* resize_half(bucket **hashtable, unsigned int *dim);



#endif
