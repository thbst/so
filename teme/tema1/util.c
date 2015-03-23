/* Lupu Cristian CC334 - Tema 1 Multi-platform Development */
#include "util.h"

#define DEBUG 1
#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif


void destroyList(List *list) {
	Node *head = *list;
	Node *p;
	while (head) {
		p = head;
		head = head->next;
		free(p->data);
		free(p);
	}
}

int addElement(List *list, const char *elem) {
	Node *head = NULL;
	Node *newNode = NULL;
	unsigned int len;
	if (strcmp("", elem) == 0)
		return FALSE;
	head = *list;
	newNode = (Node *)calloc(1, sizeof(Node));
	DIE(newNode == NULL, "Could not create new Node");

	/* Need to allocate len + 1, because of '\0' character */
	len = strlen(elem);
	newNode->data = (char *)calloc(len + 1, sizeof(char));
	DIE(newNode->data == NULL, "Could not create space for data");
	memcpy(newNode->data, elem, len + 1);
	newNode->next = NULL;

	if (head == NULL) {
		*list = newNode;
		return TRUE;
	}

	while (head->next) {
		head = head->next;
	}

	head->next = newNode;
	return TRUE;
}

int findElement(const List list, const char *elem) {
	Node *head = list;
	if (head == NULL)
		return FALSE;

	while (head) {
		if (strcmp(elem, head->data) == 0)
			return TRUE;
		head = head->next;
	}
	return FALSE;
}

int deleteElement(List *list, const char *elem) {
	Node *head = *list;
	if (head == NULL)
		return FALSE;

	/* Element is at the beginning of the list */
	if (strcmp(elem, head->data) == 0) {
		*list = (*list)->next;
		free(head->data);
		free(head);
		return TRUE;
	}

	while (head->next) {
		if (strcmp(head->next->data, elem) == 0) {
			Node *temp = head->next;
			head->next = head->next->next;
			free(temp->data);
			free(temp);
			return TRUE;
		}
		head = head->next;
	}
	return FALSE;
}

int printList(const List list, FILE *file) {
	Node *head = list;
	if (head == NULL || (head && head->data == NULL) || (head &&
			strcmp("", head->data) == 0))
		return FALSE;
	while (head) {
		fprintf(file, "%s", head->data);
		head = head->next;
		if (head)
			fprintf(file, " ");
	}
	return TRUE;
}


/* Resizes a Hash Table by multiplying the hashSize with multiplyFactor and
 * recomputing the entire table */
static int resizeTable(HashTable **table, const float multiplyFactor);

void initHashTable(HashTable **table, const unsigned int hashSize) {
	*table = (HashTable *)calloc(1, sizeof(HashTable));
	DIE((*table) == NULL, "Could not allocate enough memory for Hash Table");

	(*table)->hashSize = hashSize;
	(*table)->table = (Bucket *)calloc(hashSize, sizeof(Bucket));
	DIE((*table)->table == NULL, "Could not allocate enough memory for "\
		"buckets");
}

void destroyHashTable(HashTable **table) {
	unsigned int i;
	if ((*table) == NULL) {
		return;
	}

	for (i = 0; i < (*table)->hashSize; ++i) {
		destroyList(&(*table)->table[i].words);
	}
	free((*table)->table);
	free((*table));
}

int addWord(HashTable *table, const char *word) {
	unsigned int index;
	DIE(table == NULL, "Table inexistnt");

	index = hash(word, table->hashSize);
	if (findElement(table->table[index].words, word) == FALSE) {
		int retVal = addElement(&table->table[index].words, word);
		if (retVal == TRUE)
			return TRUE;
	}
	else
		return TRUE;

	return FALSE;
}

int deleteWord(HashTable *table, const char *word) {
	unsigned int index;
	DIE(table == NULL, "Table inexistent");

	index = hash(word, table->hashSize);
	return deleteElement(&table->table[index].words, word);
}

int findWord(const HashTable table, const char *word) {
	unsigned int index = hash(word, table.hashSize);

	return findElement(table.table[index].words, word);
}

int printBucket(
		const HashTable table,
		FILE *file,
		const unsigned int bucketIndex) {
	DIE(bucketIndex >= table.hashSize, "Index out of bounds");

	return printList(table.table[bucketIndex].words, file);
}

void printTable(const HashTable table, FILE *file) {
	unsigned int i;
	for (i = 0; i < table.hashSize; ++i) {
		if (printBucket(table, file, i) == TRUE) {
			fprintf(file, "\n");
		}
	}
}

int doubleTable(HashTable **table) {
	return resizeTable(table, 2);
}

int halveTable(HashTable **table) {
	return resizeTable(table, 0.5);
}

static int resizeTable(HashTable **table, const float multiplyFactor) {
	HashTable *newTable = NULL;
	unsigned int i;
	float newSize = (float)(*table)->hashSize * multiplyFactor;
	initHashTable(&newTable, (unsigned int)newSize);

	for (i = 0; i < (*table)->hashSize; ++i) {
		List list = (*table)->table[i].words;
		while (list) {
			addWord(newTable, list->data);
			list = list->next;
		}
	}
	destroyHashTable(table);
	*table = newTable;
	return TRUE;
}
