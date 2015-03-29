// Stan Filip Ioan 332 CA 

#include "utils.h"

struct linked_list{
	struct node *first, *last;
};

void initList(struct linked_list *L);

void destroyList(struct linked_list *L);

int isEmpty(struct linked_list *L);

void addLast(struct linked_list *L, char* word);

void removeFirst(struct linked_list *L);

void printList(FILE *f, struct linked_list *L);