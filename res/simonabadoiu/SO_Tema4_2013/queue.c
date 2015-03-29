#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "queue.h"

Queue* myfront(Queue* q)
{
	if (q == NULL) {
		return q;
	}
	return q;
}

Queue* mypush(Queue* q, unsigned int prio, tid_t thread, int id, Queue* new_thread)
{
	Queue *aux, *new;
	if (q == NULL) {
		//printf("Adauga prim elem\n");
		q = malloc(sizeof(Queue));
		if (!q)
			return q;
		q->next = NULL;
		q->prio = prio;
		q->time = 0;
		q->dev = -1;
		q->state = READY;
		q->index = id;
		//q->pause_cond = pause_cond;
		q->thread = thread;
		new_thread = q;
		return q;
	}
	
	new = malloc(sizeof(Queue));
	if (!new)
		return NULL;
	new->prio = prio;
	new->dev = -1;
	new->time = 0;
	new->state = READY;
	new->index = id;
	//new->pause_cond = pause_cond;
	new->thread = thread;
	
	if (prio > q->prio) {
		new->next = q;
		q = new;
		new_thread = new;
		//return new;
	}
	else {
		aux = q;
		while (q->next != NULL && q->next->prio >= prio)
			q = q->next;
		
		new->next = q->next;
		q->next = new;
		new_thread = new;
		return aux;
	}
	return q;
}

Queue* mypop(Queue *q)
{
	Queue* aux = q;
	q = q->next;
	return aux; 
}

void print_queue(Queue* q)
{
	if (q == NULL) {
		printf("A terminat\n");
		return;
	}
	if (q != NULL) {
		printf("%i ", q->prio);
		print_queue(q->next);
	}
}

Queue* myremove(Queue *q)
{
	Queue* aux = q;
	q = q->next;
	free(aux);
	return q;
}

Queue* myclean(Queue *q) {
	
	while (q != NULL) {
		q = myremove(q);
	}
	
	return NULL;
}

/*
int main() {
	Queue* q = NULL;
	int i, x;
	srand(time(NULL));
	for (i = 0; i < 10; i++) {
		x = rand()%3 + 1;
		q = mypush(q, x, NULL, 1);
	}
	print_queue(q);
	Queue *aux;
	while (q != NULL) {
		aux = q;
		q = q->next;
		free(aux);
	}
	print_queue(q);
}*/
