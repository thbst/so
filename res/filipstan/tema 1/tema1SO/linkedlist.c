// Stan Filip Ioan 332 CA 

#include "linkedlist.h"

void initList(struct linked_list *L){
	L->first = L->last = NULL;
}

void destroyList(struct linked_list *L){
	struct node *aux;

	while (L->first != L->last){
		aux = L->first;
		L->first = L->first->next;
		free(aux->word);
		free(aux);
	}

	if (L->last != NULL)
		free(L->last->word);
	free(L->last);
}

int isEmpty(struct linked_list *L){
	return (L->first == NULL);
}

void addLast(struct linked_list *L, char* word){
	struct node *aux;
	int word_len;

	aux = (struct node *)malloc(sizeof(struct node));
	word_len = strlen(word);
	aux->word = (char *)malloc((word_len + 1) * sizeof(char));
	memcpy(aux->word, word, word_len);
	aux->word[word_len] = '\0';
	aux->next = NULL;

	if (L->last != NULL){
		L->last->next = aux;
	}
	L->last = aux;
	if (L->first == NULL){
		L->first = L->last;
	}
}

void removeFirst(struct linked_list *L){
	if (L->first != NULL){
		struct node *aux;
		aux = L->first->next;
		if (L->first == L->last){
			L->last = NULL;
		}
		free(L->first->word);
		free(L->first);
		L->first = aux;
	}
}

void printList(FILE *f, struct linked_list *L){
	if (L->first != NULL){
		struct node *aux;
		aux = L->first;
		while (aux != NULL){
			fprintf(f, "%s ", aux->word);
			aux = aux->next;
		}
		fprintf(f, "\n");
	}
}