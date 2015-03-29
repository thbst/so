/* Badoiu Simona Andreea 331CB - Tema0 HashTable */
#ifndef EXECUTA_H_
#define EXECUTA_H_


#include "operatii.h"

/*returneaza tipul de operatie corespunzator sirului de caractere primit
ca parametru */
unsigned int tipOperatie(char *c);
/* citeste comenzi din fisier si apeleaza executa pentru fiecare comanda */
void citesteFisier(char *fisier, bucket **hashtable, unsigned int *dim);
/* citeste comenzi de la stdin si apeleaza executa pentru fiecare comanda */
void citesteStdin(bucket **hashtable, unsigned int *dim);
/* face prelucrarile necesare pe sirul de caractere result si apeleaza
functia de care este nevoie pentru a executa operatia ceruta prin variabila
tip_operatie */
void executa(unsigned int tip_operatie,
			char* result,
			bucket **hashtable,
			unsigned int *dim);

#endif
