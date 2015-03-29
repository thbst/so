#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "so_scheduler.h"
#include "queue.h"

/* priority queues containing all the threads */
static Queue* all_threads = NULL;
static Queue* ready_threads = NULL;
static Queue *running = NULL, *new_thread = NULL;
static Queue *waiting_threads = NULL;
static Queue *terminated_threads = NULL;

static unsigned int max_io, index1 = 0, global_time = 0;
int timp = 0;
pthread_cond_t *devices;
pthread_cond_t *conds;
pthread_mutex_t* mutex;
pthread_mutex_t aux_mutex;
int *stop = NULL;
/* running - the address of the item of the queue corresponding to the running thread 
   first_thred - the address of the structure containing the first thread
*/
static Queue *first_thread = NULL;
/* is 1 if no thread was created yet */
static int first = 2;

struct params {
	unsigned int prio, ind;
	so_handler *handler;
};

static void stop_thread(unsigned int index, Queue *thread, int repush)
{
	int rc;
	rc = pthread_mutex_lock(&mutex[index]);
	if (rc != 0) {
		printf("pthread_mutex_lock");
		return;
	}
		
	/* stop the running thread */
	stop[index] = 1;
	
	thread->state = READY;
	thread->time = 0;
	if (repush == 1)
		ready_threads = mypush(ready_threads, running->prio, running->thread, running->index, new_thread);
	
	while (stop[index] == 1) {
		rc = pthread_cond_wait(&conds[index], &mutex[index]);
		if (rc != 0) {
			printf("pthread_cond_wait");
			return;
		}
	}
		
	/* unlock the mutex */
	rc = pthread_mutex_unlock(&mutex[index]);
	if (rc != 0) {
		printf ("pthread_mutex_unlock\n");
		return;
	}
}

static void wake_thread(unsigned int index, Queue *thread)
{
	int rc;
	Queue *aux;
	
	rc = pthread_mutex_lock(&mutex[index]);
	if (rc != 0) {
		printf("pthread_mutex_lock\n");
		return;
	}
	
	stop[index] = 0;
	thread->state = RUNNING;
	thread->time = 0;
	/*
	 remove the first thread from ready_threads, because always this thread
	 will be the running thread
	*/
	aux = ready_threads;
	ready_threads = ready_threads->next;
	free(aux);
	
	rc = pthread_cond_signal(&conds[index]);
	if (rc != 0) {
		printf("pthread_cond_signal\n");
		return;
	}
	
	rc = pthread_mutex_unlock(&mutex[index]);
	if (rc != 0) {
		printf("pthread_mutex_unlock\n");
		return;
	}
}

static void round_robin(Queue* q)
{
	/* the thread with the highest priority is the first element on the queue */
	Queue *aux = q;
	
	/* stop the running thread */
	if (running != NULL) {
		stop_thread(running->index, running, 1);
	}
	
	/*
	 - aux will become the running thread
	 - remove thread from ready_threads and running will keep the running thread
	*/
	if (aux != NULL) {
		running = aux;
		//ready_threads = ready_threads->next;	
		/* wake up the new scheduled thread */
		wake_thread(running->index, running);
	}
}

/*
 start the thread with the highest priority
 from the priority queue
*/
static void schedule()
{
	/* daca a trecut o cuanta, trebuie sa reprogramam threadurile */
	if (running == NULL) {
		round_robin(ready_threads);
	}
	else {
		if (running->time == timp) {
			round_robin(ready_threads);
		}
	}
}

DECL_PREFIX int so_init(unsigned cuanta, unsigned io)
{
	printf("So init \n");
	/* sanity  checks */
	if (cuanta <= 0)
		return -1;
	if (io < 0)
		return -1;
	
	timp = cuanta;
	max_io = io;
	
	devices = calloc(io, sizeof(int));
	if (!devices) {
		printf("Calloc\n");
		return -1;
	}
	
	/* init condition variables corresponding to devices */
	/*for (i = 0; i < max_io; i++) {
		pthread_cond_init(&devices[i], NULL);
	}
*/
	pthread_mutex_init(&aux_mutex, NULL);
	return 0;
}

void* start_thread(void* args)
{
	unsigned int ind;
	Queue *new = new_thread, *aux;
	int rc;
	struct params *param = (struct params*)args;
	ind = param->ind;
	
	/* wait until the thread is added to the ready_queue */
	rc = pthread_mutex_lock(&aux_mutex);
	if (rc != 0) {
		printf("pthread_mutex_lock");
		return NULL;
	}
	
	if (running != NULL) {
		/*
		 if a thread with higher priority was created, reschedule */
		if (param->prio > running->prio) {
			stop_thread(running->index, running, 1);
			
			running = new;
			/* delete the thread from ready_threads */
			wake_thread(running->index, running);
		}
		else {
			/* it the time has expired */
			if (running->time == timp) {
				round_robin(ready_threads);
			}
			else {
				/* if we have to stop the thread */
				/* don't need to repush the thread, because it's already in the queue */
				stop_thread(new->index, new, 0);
			}
		}
	}
	/* if running == NULL, we certainly won't stop the thread */
	else {
		if (ind > 0) {
			/* try to schedule the thread */
			schedule();
		}
	}
	
	rc = pthread_mutex_unlock(&aux_mutex);
	if (rc != 0) {
		printf("pthread_mutex_unlock\n");
		return;
	}
	/* run the function received as parameter */
	param->handler(param->prio);

	running->state = TERMINATED;
	terminated_threads = mypush(terminated_threads, running->prio, running->thread, running->index, aux);
	stop_thread(running->index, running, 0);	
	
	return NULL;
}

/*
 * creates a new so_task_t and runs it according to the scheduler
 * + handler function
 * + priority
 * returns: tid of the new task if successful or INVALID_TID
 */
DECL_PREFIX tid_t so_fork(so_handler *handler, unsigned prio)
{
	printf("Incepe fork\n");
	int rc;

	if (handler == 0) {
		return INVALID_TID;
	}
	
	/* TODO sa fac un vector de parametrii pentru a ii elibera la sfarsit */
	tid_t fir;
	struct params* param = malloc(sizeof(struct params));
	param->prio = prio;
	param->handler = handler;
	param->ind = index1;
	
	stop = realloc(stop, (index1 + 1)*sizeof(int));
	if (!stop)
		return INVALID_TID;
	stop[index1] = 1;
	
	conds = realloc(conds, (index1 + 1)*sizeof(pthread_cond_t));
	if (!conds)
		return INVALID_TID;
	pthread_cond_init(&conds[index1], NULL);
	
	mutex = realloc(mutex, (index1 + 1)*sizeof(pthread_mutex_t));
	if (!mutex)
		return INVALID_TID;
	
	/* lock the mutex */
	rc = pthread_mutex_lock(&aux_mutex);
		if (rc != 0) {
			printf("pthread_mutex_lock");
			return -1;
	}
	
	/* create the thread - this thread will be locked until the mutex is unlocked */
	if (pthread_create(&fir, NULL, &start_thread, param)) {
		return INVALID_TID;
	}
	
	/* keep the first thread in some variable */
	if (index1 == 0) {
		first_thread = malloc(sizeof(Queue));
		if (!first_thread)
			return -1;
		first_thread->prio = prio;
		first_thread->index = index1;
		first_thread->dev = -1;
		first_thread->state = READY;
		first_thread->time = 0;
		first_thread->thread = fir;
	}
	else {
		/* keep the thread in a priority queue - the queue will be used for scheduling */
		ready_threads = mypush(ready_threads, prio, fir, index1, new_thread);
		all_threads = mypush(all_threads, prio, fir, index1, new_thread);
	}
	
	rc = pthread_mutex_unlock(&aux_mutex);
	if (rc != 0) {
		printf("pthread_mutex_unlock\n");
		return;
	}
		
	if (index1 != 0)
		global_time++;
		
	if (running != NULL) {
		running->time += 1;
		schedule();
	}
	
	index1++;
	
	return fir;
}


/*
 * waits for an IO device
 * + device index
 * returns: -1 if the device does not exist or 0 on success
 */
DECL_PREFIX int so_wait(unsigned event)
{
	printf("So wait apelat\n");
	if (event >= max_io || event < 0) {
		return -1;
	}
	
	global_time++;
	
	if (running != NULL) {
		running->time += 1;
		schedule();
	}
	printf("So wait terminat\n");
	return 0;
}

/*
 * signals an IO device
 * + device index
 * return the number of tasks woke or -1 on error
 */
DECL_PREFIX int so_signal(unsigned event)
{
	printf("So signal apelat\n");	
	if (event >= max_io || event < 0) {
		return -1;
	}
	
	global_time++;
	if (running != NULL) {
		running->time += 1;
		schedule();
	}
	printf("So signal terminat\n");
	return 0;
}

/*
 * does whatever operation
 */
DECL_PREFIX void so_exec()
{
	int i = 0;
	i++;
	
	global_time++;
	printf("global_time %i\n", global_time);
	if (running != NULL) {
		running->time += 1;
		printf("running->time = %i\n", running->time);
		schedule();
	}
	printf("global_time copy %i\n", global_time);
}

/*
 * destroys a scheduler
 */
DECL_PREFIX void so_end()
{
	/* here we have to go through all the TERMINATED threads and join them */
	printf("Intra in end\n");
	Queue *aux = all_threads;
	while (aux != NULL) {
		if (pthread_join(aux->thread, NULL))
			perror("pthread join\n");
		aux = aux->next;
	}
	printf("all the threads have finished\n");
	if (all_threads != NULL)
		all_threads = myclean(all_threads);
	printf("aici \n");
	//if (pthread_join(first_thread->thread, NULL))
		//perror("pthread join\n");
	printf("se termina \n");
}
