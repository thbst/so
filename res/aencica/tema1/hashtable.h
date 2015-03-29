/*
	Encica Alexandru
	Grupa 332CC
	Tema 1 SO
*/

#ifndef HASHTABLE_H_

#define HASHTABLE_H_

typedef struct _Entry {
	char *word;
	struct _Entry *next;
} Entry;

typedef struct _HashTable{
	unsigned int size;
	struct _Entry **table;
} HashTable;
/**
 * Functie ce intitalizeaz un hashtable; 
 * Primeste ca parametru size-ul acestuia si aloc spatiu in memorie.
 * @return intoarce un hashtable.
 */
HashTable *init(unsigned int size);

/**
 * Functie ce adauga un cuvant nou intr-un hashtable;
 * Primeste ca parametru hashtable-ul si cuvantul ce va trebui
 * adaugat. Adaugare se face la sfarsit in bucket.
 */
void add(HashTable *ht, char *word);

/**
 * Functie ce sterge un cuvant dintr-un hashtable;
 * Primeste ca parametrii hashtable-ul si cuvantul ce va 
 * trebui sters.
 */
void delete(HashTable *ht, char *word);

/**
 * Functie ce cauta un cuvant intr-un hashtable.
 * Primeste ca parametrii hashtable-ul si cuvantul
 * @returns 1 daca cuvantul exista in hashtable, sau 
 * 			0 daca cuvantul nu exista in hashtable.
 */
int find(HashTable *ht, char *word);

/**
 * Sterge toate intrarile dintr-un hashtable;
 * Primeste ca parametru hashtable-ul.
 */
void clear(HashTable *ht);

/**
 * Functie folosita pentru add. Cauta un cuvant intr-un hashtable 
 * @returns NULL daca exista 
 *			pointer la ultimul element din bucket daca nu exista
 */
Entry *not_exists(HashTable *ht, char *word, unsigned int bucket);

/**
 * Printeaza elementele dintr-un bucktet al unui hashtable intr-un fisier
 * Primeste ca parametetrii hashtablelul, indexul bucket-ului si
 * fisierul de iesire. 
 */
void print_bucket(HashTable *ht,unsigned int index, FILE *out_file);

/**
 * Printeaza elementele dintr-un hashtable intr-un fisier
 * Primeste ca parametetrii hashtablelul si fisierul de iesire. 
 */
void print(HashTable *ht, FILE *out_file);

/**
 * Copiaza elementele dintr-un bucket al unui hashtable in alt hashtable
 * Primeste ca parametrii cele 2 hashtable-uri si indexul bucket-ului din 
 * care se doresteste copierea 
 */
void copy_elements(HashTable *ht, HashTable *htn, unsigned int bucket);

/**
 * Redimensioneaza un hashtable la o dimensiune dorita
 * Primeste ca parametrii hashtable-ul si noua dimensiune
 * @returns un nou hashtable cu dimensiunea dorita.
 */
HashTable *resize(HashTable *ht, unsigned int size);

/**
 * Dubleaza dimensiunea unui hashtable
 * Primeste ca parametrii hashtable-ul
 * @returns un nou hashtable cu noua dimensiune
 */
HashTable *resize_double(HashTable *ht);

/**
 * Injumatateste dimensiunea unui hashtable
 * Primeste ca parametrii hashtable-ul
 * @returns un nou hashtable cu noua dimensiune
 */
HashTable *resize_halve(HashTable *ht);

#endif