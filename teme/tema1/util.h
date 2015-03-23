/* Lupu Cristian CC334 - Tema 1 Multi-platform Development */
#ifndef _UTIL_H
#define _UTIL_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#define DBG 0
#define BUFSZ 20001
#define CMDSZ 32

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

enum Bool {
	FALSE = 0,
	TRUE = 1
};

/* Structure for a Node of a list */
typedef struct Node_ {
	char *data;
	struct Node_ *next;
}Node;

typedef Node* List;

/* Deallocates resources and destroys the list */
void destroyList(List *list);

/* Inserts an element to a list. Returns TRUE on success, FALSE otherwise */
int addElement(List *list, const char *elem);

/* Searches an element in a list. Returns TRUE if found, FALSE otherwise */
int findElement(const List list, const char *elem);

/* Deletes an element in a list. Returns TRUE if element was found and deleted,
 * FALSE otherwise */
int deleteElement(List *list, const char *elem);

/* Print List to file. Return TRUE if List is empty, FALSE otherwise. */
int printList(const List list, FILE *file);

/* Structure that holds a Pair key-value for words with the same hash */
typedef struct Bucket_ {
	List words;
}Bucket;

/* Structure for a Hash Table */
typedef struct HashTable_ {
	unsigned int hashSize;
	Bucket *table;
}HashTable;

/* Creates a Hash Table with a given hashSize for the hash function */
void initHashTable(HashTable **table, const unsigned int hashSize);

/* Destroys a Hash Table and releases allocated memory. */
void destroyHashTable(HashTable **table);

/* Adds a word to the Hash Table. If it already exists, it does nothing.
 * Returns TRUE if word is added successfully, FALSE otherwise */
int addWord(HashTable *table, const char *word);

/* Removes a word from the Hash Table. The word could be inexistent. Returns
 * TRUE if element is found and deleted, FALSE otherwise */
int deleteWord(HashTable *table, const char *word);

/* Searches for a word in the Hash Table. Returns TRUE if found, FALSE
 * otherwise */
int findWord(const HashTable table, const char *word);

/* Prints bucket to a FILE. Each word is separated by space. Returns TRUE if
 * Bucket elements exist, FALSE otherwise. */
int printBucket(
		const HashTable table,
		FILE *file,
		const unsigned int bucketIndex);

/* Prints HashTable to a FILE. Each bucket is printed on a separate line. */
void printTable(const HashTable table, FILE *file);

/* Doubles the size of the hash function of the Hash Table. Recomputes the
 * entire table by creating a new one. Returns TRUE on success, FALSE otherwise
 * */
int doubleTable(HashTable **table);

/* Halves the size of the hash function of the Hash Table. Recomputes the
 * entire table by creating a new one. Returns TRUE on success, FALSE otherwise
 * */
int halveTable(HashTable **table);


#endif
