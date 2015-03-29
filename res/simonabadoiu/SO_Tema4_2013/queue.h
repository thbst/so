#ifndef _QUEUE_H_
#define _QUEUE_H_

//#include "so_scheduler.h"
#include <pthread.h>

#define NEW 0
#define READY 1
#define RUNNING 2
#define WAITING 3
#define TERMINATED 4

typedef pthread_t tid_t;
/*
 prio - the priority of this thread
 state - gives the state of this thread - READY, NEW, etc
 time - the time spend running
 thread - thread id
 dev - the device this thread is waiting for
 pause_cond - condition variable used to pause the thread after it's time finishes
*/
typedef struct queue_item {
	unsigned int prio;
	int index;
	int dev;
	unsigned int state, time;
	tid_t thread;
	struct queue_item* next;
} Queue;

Queue* myfront(Queue* q);

Queue* mypush(Queue* q, unsigned int prio, tid_t thread, int id, Queue* new_thread);

Queue* mypop(Queue *q);

void print_queue(Queue* q);

Queue* myclean(Queue* q);

#endif
