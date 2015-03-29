/* Nume Prenume Grupa - Tema 1 Multi-platform Development */
#include "util.h"

#define DEBUG 1
#ifdef DEBUG
#define dprintf(msg,...) \
	printf("[%s]:%d " msg, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif

void clear_list(List *list) {
	Node *node_p;
	Node *top = *list;

	for (;top;) {
		node_p = top;
		top = top->next;
		free(node_p->data);
		free(node_p);
	}
}

int add_member(List *list, char *member) {
	Node *top = NULL;
	Node *node_new = NULL;

	if (strcmp("", member) == 0)
		return 0;

	top = *list;
	node_new = (Node *)malloc(sizeof(Node));
	DIE(NULL == node_new, "Error creating new node");

	node_new->data = (char *)calloc(strlen(member) + 1, sizeof(char));
	DIE(NULL == node_new->data, "Error allocating memory");

	memcpy(node_new->data, member, strlen(member) + 1);
	node_new->next = NULL;

	if (top == NULL) {
		*list = node_new;
		return 1;
	}

	for (;top->next;)
		top = top->next;

	top->next = node_new;
	return 1;
}

int find_member(List list, char *member) {
	Node *top = list;
	if (top == NULL)
		return 0;

	for (;top;) {
		if (strcmp(member, top->data) == 0)
			return 1;
		top = top->next;
	}
	return 0;
}

int remove_member(List *list, char *member) {
	Node *top = *list;
	if (top == NULL)
		return 0;

	if (strcmp(member, top->data) == 0) {
		*list = (*list)->next;
		free(top->data);
		free(top);
		return 1;
	}

	for (;top->next;) {
		if (strcmp(member, top->next->data) == 0) {
			Node *aux = top->next;
			top->next = top->next->next;
			free(aux->data);
			free(aux);
			return 1;
		}
		top = top->next;
	}
	return 0;
}

int print_node(FILE *file, List list) {
	Node *top = list;
	if (top == NULL)
		return 0;

	if (top && top->data == NULL)
		return 0;

	if (top && strcmp("", top->data) == 0)
		return 0;

	for (;top;) {
		fprintf(file, "%s", top->data);
		top = top->next;
		if (top)
			fprintf(file, " ");
	}
	return 1;
}

void init_hash(HashTable **hash_t, int hash_length) {
	*hash_t = (HashTable *)malloc(sizeof(HashTable));
	DIE((*hash_t) == NULL, "Error allocating memory for hash");

	(*hash_t)->hash_length = hash_length;
	(*hash_t)->hash_t = (Bucket *)calloc(hash_length, sizeof(Bucket));
	DIE((*hash_t)->hash_t == NULL, "Error allocating memory for buckets");
}

void free_hash(HashTable **hash_t) {
	int i=0;
	if ((*hash_t) == NULL)
		return;

	while (i < (*hash_t)->hash_length) {
		clear_list(&(*hash_t)->hash_t[i].words);
		i++;
	}
	free((*hash_t)->hash_t);
	free((*hash_t));
}

int add_word(HashTable *hash_t, char *word) {
	int i;
	DIE(hash_t == NULL, "Error during hash_t call");

	i = hash(word, hash_t->hash_length);
	if (find_member(hash_t->hash_t[i].words, word) == 0) {
		return add_member(&hash_t->hash_t[i].words, word);
	}

	return 0;
}

int remove_word(HashTable *hash_t, char *word) {
	int i;
	DIE(hash_t == NULL, "Error during hash_t call");

	i = hash(word, hash_t->hash_length);
/*	dprintf("removed %s", word);*/
	return remove_member(&hash_t->hash_t[i].words, word);
}

int print_bucket(FILE *file,HashTable hash_t,int key) {
	DIE(key >= hash_t.hash_length, "Error in buffer size");

	return print_node(file, hash_t.hash_t[key].words);
}

void print_hash(FILE *file, HashTable hash_t) {
	int i=0;
	while (i < hash_t.hash_length) {
		if (print_bucket(file, hash_t, i) == 1) {
			fprintf(file, "\n");
		}
		i++;
	}
}

int find_word(char *word, HashTable hash_t) {
	int i = hash(word, hash_t.hash_length);

	return find_member(hash_t.hash_t[i].words, word);
}

static int resize_hash(HashTable **hash_t, float multiplyFactor) {
	HashTable *newTable = NULL;
	int i=0;
	float newSize = (float)(*hash_t)->hash_length * multiplyFactor;
	init_hash(&newTable, (int)newSize);

	while (i < (*hash_t)->hash_length) {
		List list = (*hash_t)->hash_t[i].words;
		for (;list;) {
			add_word(newTable, list->data);
			list = list->next;
		}
	i++;
	}
	free_hash(hash_t);
	*hash_t = newTable;
	return 1;
}


int resize_double(HashTable **hash_t) {
	return resize_hash(hash_t, 2);
}

int resize_half(HashTable **hash_t) {
	return resize_hash(hash_t, 0.5);
}

void print_word(FILE *f, int result) {
	if (result == 1)
		fprintf(f, "True\n\n");
	else if (result == 0)
		fprintf(f, "False\n\n");
}

void run_operation(HashTable **hashTable, char buf[3][BUFSIZE]) {
	char command[CMDSIZE];
	char *token;
	int bucket, size, result;
	FILE *f;

	token = strtok(buf[0], " \n");
	if (token == NULL || strcmp(token, "") == 0)
		return;
	strcpy(command, token);

	token = strtok(NULL, " \n");
	if (token) {
		strcpy(buf[1], token);
	}
	token = strtok(NULL, " \n");
	if (token) {
		strcpy(buf[2], token);
	}

	if (strcmp(command, "print_bucket") == 0) {
		bucket = atoi(buf[1]);
		if (strcmp(buf[2], "") && buf[2] != 0) {
			f = fopen(buf[2], "a");
			DIE(f == NULL, "Error opening file");
			if (print_bucket(f,**hashTable, bucket) == 1) {
				fprintf(f, "\n\n");
				dprintf("");
			}
			else
				fprintf(f, "\n");
			fclose(f);
		}
		else {
			if (print_bucket(stdout,**hashTable, bucket) == 1) {
				fprintf(stdout, "\n\n");
				dprintf("");
			}
			else
				fprintf(stdout, "\n");
		}

		return;
	}
	if (strcmp(command, "print") == 0) {
		if (strcmp(buf[1], "") && buf[1] != 0) {
			f = fopen(buf[1], "a");
			DIE(f == NULL, "Error opening file");
			print_hash(f, **hashTable);
			fprintf(f, "\n");
			fclose(f);
		}
		else {
			print_hash(stdout, **hashTable);
			printf("\n");
		}
		return;
	}
	if (strcmp(command, "find") == 0) {
		if (buf[1]) {
			result = find_word(buf[1], **hashTable);
			if (buf[2] && strcmp(buf[2], "") != 0) {
				f = fopen(buf[2], "a");
				DIE(f == NULL, "Error opening file");
				print_word(f, result);
				fclose(f);
			}
			else {
				print_word(stdout, result);
			}
		}
		return;
	}
	if (strcmp(command, "remove") == 0) {
		remove_word(*hashTable, buf[1]);
		return;
	}
	if (strcmp(command, "add") == 0) {
		add_word(*hashTable, buf[1]);
		return;
	}
	if (strcmp(command, "clear") == 0) {
		size = (*hashTable)->hash_length;
		free_hash(hashTable);
		init_hash(hashTable, size);
		return;
	}
	if (strcmp(command, "resize") == 0) {
		if (strcmp("halve",buf[1]) == 0) {
			resize_half(hashTable);
		}
		else if (strcmp("double",buf[1]) == 0) {
			resize_double(hashTable);
		}
		return;
	}
}

void parse_file(FILE *file, HashTable **hashTable) {
	char buf[3][BUFSIZE];
	for (;fgets(buf[0], BUFSIZE, file) != NULL;) {
		run_operation(hashTable, buf);
		memset(buf, 0, 3 * BUFSIZE * sizeof(char));
	}
}
