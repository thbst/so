/*
	Encica Alexandru
	Grupa 332CC
	mail: alexandru.encica@cti.pub.ro
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<string.h>

#include "hash.h"
#include "hashtable.h"
#define BUFFSIZE 20000
#define DELIMS " \n\r\t"

#ifndef UTILS_H_

#define UTILS_H_

#define DIE(assertion, call_description)		\
	do {										\
		if (assertion) {						\
			fprintf(stderr, "(%s, %d): ",		\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);					\
		}										\
	} while(0)

/**
 * Parseaza o comanda si in functie de comanda primita 
 * ca parametru va face anumite operatii pe hashtable.
 * Primeste ca parametru un hashtable-ul si comanda.
 * @returns hashtable-ul actualizat.
 */
HashTable *parseLine(HashTable *ht, char *line);

/**
 * Parcurge un fisier linie cu linie si trimite comanda de pe o 
 * linie catre parseLine.
 * Primeste ca parametru un hashtable-ul si un fisier de intrare.
 * @returns hashtable-ul actualizat.
 */
HashTable *parseFile(HashTable *ht, FILE *in_file);

#endif