// Stan Filip Ioan 332 CA 

#include "hashtable.h"

void initHashtable(struct hashtable *Hashtable, unsigned int _size){
	unsigned int i;

	Hashtable->table = (struct linked_list *)malloc(_size * sizeof(struct linked_list));
	Hashtable->size = _size;

	for (i = 0; i < _size; i++){
		initList(&Hashtable->table[i]);
	}
}

void clearHashtable(struct hashtable *Hashtable){
	unsigned int i;
	for (i = 0; i < Hashtable->size; i++){
		destroyList(&Hashtable->table[i]);
		initList(&Hashtable->table[i]);
	}
}

void destroyHashtable(struct hashtable *Hashtable){
	clearHashtable(Hashtable);
	free(Hashtable->table);
	Hashtable->size = 0;
}

void put(struct hashtable *Hashtable, char* word){
	struct node *aux;
	unsigned int hkey = hash(word, Hashtable->size);

	aux = Hashtable->table[hkey].first;

	while (aux != NULL){
		if (strcmp(aux->word, word) == 0){
			break;
		}
		aux = aux->next;
	}

	if (aux == NULL){
		addLast(&Hashtable->table[hkey], word);
	}
}

int find(struct hashtable *Hashtable, char* word){
	struct node *aux;
	unsigned int hkey = hash(word, Hashtable->size);

	aux = Hashtable->table[hkey].first;
	while (aux != NULL){
		if (strcmp(aux->word, word) == 0){
			break;
		}
		aux = aux->next;
	}

	if (aux != NULL){
		return 1;
	}

	return 0;
}

void removeWord(struct hashtable *Hashtable, char* word){
	struct node *aux, *aux2;
	unsigned int hkey = hash(word, Hashtable->size);

	aux = Hashtable->table[hkey].first;

	if (aux != NULL){
		if (strcmp (aux->word, word) == 0){
			removeFirst(&Hashtable->table[hkey]);
		} else {
			while (aux->next != NULL){
				if (strcmp (aux->next->word, word) == 0){
					aux2 = aux->next;
					if (aux2 == Hashtable->table[hkey].last){
						Hashtable->table[hkey].last = aux;
					}
					aux->next = aux2->next;
					free(aux2->word);
					free(aux2);

					break;
				}
				aux = aux->next;
			}
		}
	}
}

struct hashtable* resizeHashtable(struct hashtable *Hashtable, float f){
	struct hashtable *H_aux;
	struct node *aux;
	unsigned int i, new_size;

	H_aux = (struct hashtable *)malloc(sizeof(struct hashtable));
	new_size = (unsigned int)(Hashtable->size * f);
	initHashtable(H_aux, new_size);

	for (i = 0; i < Hashtable->size; i++){
		aux = Hashtable->table[i].first;
		while (aux != NULL){
			put(H_aux, aux->word);
			aux = aux->next;
		}
	}
	destroyHashtable(Hashtable);
	free(Hashtable);
	return H_aux;
}

void printHashtable(FILE *f, struct hashtable *Hashtable){
	unsigned int i;

	for (i = 0; i < Hashtable->size; i++){
		printList(f, &Hashtable->table[i]);
	}
	fprintf(f, "\n");
}