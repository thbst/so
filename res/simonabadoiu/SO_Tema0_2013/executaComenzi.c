/* Badoiu Simona Andreea 331CB - Tema0 HashTable */
#include <stdio.h>
#include <stdlib.h>

#include "executaComenzi.h"

#define SIZE 20000

#define ADD 1
#define RMV 2
#define CLEAR 3
#define FIND 4
#define PRINT_B 5
#define PRINT 6
#define RESIZE 7

/* Folositoare pentru switch-ul folosit in functia executa */
unsigned int tipOperatie(char *c) {
	if (strcmp(c, "add") == 0) {
		return ADD;
	}
	if (strcmp(c, "remove") == 0) {
		return RMV;
	}
	if (strcmp(c, "clear") == 0) {
		return CLEAR;
	}
	if (strcmp(c, "find") == 0) {
		return FIND;
	}
	if (strcmp(c, "print_bucket") == 0) {
		return PRINT_B;
	}
	if (strcmp(c, "print") == 0) {
		return PRINT;
	}
	if (strcmp(c, "resize") == 0) {
		return RESIZE;
	}
	return 0;
}

/* citeste fiecare comanda din fisier si apeleaza
executa(parametrii corespunzatori) */
void citesteFisier(char *fisier, bucket **hashtable, unsigned int *dim) {
	FILE* fis_in;
	int ret;
	/* in result avem rezultatul dupa strtok */
	char s[SIZE], *result;
	unsigned int tip_operatie;
	fis_in = fopen(fisier, "r");
	DIE(fis_in == NULL, "Nu s-a putut deschide fisierul!\n");
	/* ret - folosit pentru a verifica daca s-a deschis sau nu fisierul */
	ret = fscanf(fis_in, "%[^\n]\n", s);
	while (ret == 1) {
		/* in result vom avea numele comenzii ce trebuie executata */
		result = strtok(s, " \n");
		/* determina tipul operatiei, in functie de comanda citita */
		tip_operatie = tipOperatie(result);
		/* executa operatia, apeland functia executa */
		executa(tip_operatie, result, hashtable, dim);
		/* citeste urmatoarele comenzi din fisier */
		ret = fscanf(fis_in, "%[^\n]\n", s);
	}
	fclose(fis_in);
}

/* citeste fiecare comanda de la stdin si apeleaza
executa(parametrii corespunzatori) */
void citesteStdin(bucket **hashtable, unsigned int *dim) {
	char s[SIZE], *result;
	unsigned int tip_operatie;
	while (1) {
		result = NULL;
		fgets(s, SIZE, stdin);
		if (strlen(s) > 1) {
			/* in result vom avea numele comenzii ce trebuie executata */
			result = strtok(s, " \n");
			tip_operatie = tipOperatie(result);
			executa(tip_operatie, result, hashtable, dim);
		}
	}
}

/* executa comanda pe care o primeste ca parametru, in tip_operatie,
determinan restul de parametrii asociati comenzii din string-ul result
(adica din restul comenzii citie din fisier/stdin) */
void executa(unsigned int tip_operatie,
			char* result,
			bucket **hashtable,
			unsigned int *dim)
{
	int index;
	char *fis_out = NULL, *cuvant;
	switch (tip_operatie) {
		case ADD:
			/* in result se va afla cuvantul de adaugat */
			result = strtok(NULL, " \n");
			add(result, hashtable, *dim);
			break;
		case RMV:
			/* in result se va afla cuvantul ce trebuie sters */
			result = strtok(NULL, " \n");
			rmv(result, hashtable, *dim);
			break;
		case CLEAR:
			clear(hashtable, *dim);
			break;
		case FIND:
			/* luam intai cuvantul di result si apoi fisierul in care trebuie
			sa scriem rezultatul, in caz ca l-am primit in comanda */
			result = strtok(NULL, " \n");
			cuvant = result;
			result = strtok(NULL, " \n");
			if (result != NULL)
				fis_out = result;
			find(cuvant, *hashtable, *dim, fis_out);
			break;
		case PRINT_B:
			/* din result luam indexul bucketului ce trebuie afisat si
			fisierul in care trbuie sa afisam, in caz ca l-am primit in
			comanda citita */
			result = strtok(NULL, " \n");
			index = atoi(result);
			result = strtok(NULL, " \n");
			if (result != NULL)
				fis_out = result;
			print_bucket(index, *hashtable, *dim, fis_out);
			break;
		case PRINT:
			/* verificam daca am primit un fisier in care trebuie sa afisam */
			result = strtok(NULL, " \n");
			if (result != NULL)
				fis_out = result;
			print_all(*hashtable, *dim, fis_out);
			break;
		case RESIZE:
			/* in comanda trebuie sa primit double sau halve si, in functie
			de ce primim, apelam una din cele doua functii de resize */
			result = strtok(NULL, " \n");
			if (strcmp(result, "double") == 0)
				*hashtable = resize_double(hashtable, dim);
			else
				*hashtable = resize_half(hashtable, dim);
			break;
	}
}

