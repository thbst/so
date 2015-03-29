// Stan Filip Ioan 332 CA 

#include "linkedlist.h"
#include "hash.h"

struct hashtable{
	struct linked_list *table;
	unsigned int size;
};

void initHashtable(struct hashtable *Hashtable, unsigned int _size);

void clearHashtable(struct hashtable *Hashtable);

void destroyHashtable(struct hashtable *Hashtable);

void put(struct hashtable *Hashtable, char* word);

int find(struct hashtable *Hashtable, char* word);

void removeWord(struct hashtable *Hashtable, char* word);

struct hashtable* resizeHashtable(struct hashtable *Hashtable, float f);

void printHashtable(FILE *f, struct hashtable *Hashtable);