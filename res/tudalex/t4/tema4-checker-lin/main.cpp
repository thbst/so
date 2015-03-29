#include "so_scheduler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>


#define DIE(assertion, message) \
do { \
 if (assertion) { \
  perror(message); \
  exit(-1); \
 } \
} while (0)

//Am alocat tot ce imi trebuie in memoria globala

std::vector<so_handler*> handlers;
std::vector<sem_t*> sem;
std::vector<unsigned> prio;
std::vector<char> waiting;
std::vector<std::vector<unsigned> > dev_q;
std::vector<unsigned> tree[SO_MAX_PRIO + 1];
std::vector<tid_t> threads;

unsigned quantum;
unsigned io_max;
unsigned thread_cnt;
unsigned current_thread;
unsigned current_quantum;
unsigned current_pos[SO_MAX_PRIO + 1];
const char debug = 0;

void scheduler(int wait = 1);

int so_init(unsigned _quantum, unsigned _io_max) {
	if (_quantum == 0)
		return -1;
	io_max = _io_max;
	dev_q.resize(io_max + 1);
	quantum = _quantum;
	thread_cnt = 0;

	return 0;
}

void* start_thread(void * thread_id) {
	if (debug)
		fprintf(stderr, "waiting for sem %p\n", sem[(unsigned) thread_id]);

	DIE(sem_wait(sem[(unsigned) thread_id]) == -1, "wait");

	if (debug)
		fprintf(stderr, "handler param %p %u\n", handlers[(unsigned) thread_id],
				prio[(unsigned) thread_id]);

	handlers[(unsigned) thread_id](prio[(unsigned) thread_id]);

	//Setam thread-ul cu waiting ca sa fie ignorat
	waiting[(unsigned) thread_id] = 1;
	current_quantum = 0;
	scheduler(0);

	return NULL;
}

tid_t so_fork(so_handler *handler, unsigned priority) {
	if (handler == NULL)
		return INVALID_TID ;

	if (debug)
		fprintf(stderr, "handler %p\n", handler);
	sem_t *r;
	int rc;
	tid_t tid;
	r = (sem_t*) calloc(1, sizeof(sem_t));
	if (r == NULL)
		return INVALID_TID ;

	rc = sem_init(r, 0, 0);
	if (rc == -1)
		return INVALID_TID ;

	sem.push_back(r);


	tree[priority].push_back(thread_cnt);
	waiting.push_back(0);
	handlers.push_back(handler);
	prio.push_back(priority);
	++thread_cnt;

	//Creeam thread-ul nou
	rc = pthread_create(&tid, NULL, start_thread, (void*) (thread_cnt - 1));
	if (rc != 0)
		return INVALID_TID ;

	threads.push_back(tid);
	--current_quantum;
	if (thread_cnt == 1) {
		//Primul thread pornit
		current_thread = thread_cnt-1;
		current_quantum = quantum;
		DIE(sem_post(sem[thread_cnt-1]) == -1,"post");
	} else
		scheduler();
	return tid;
}

int so_signal(unsigned dev) {
	if (dev >= io_max)
		return -1;
	int tasks = dev_q[dev].size();

	//Resetam waiting-ul
	for (unsigned i = 0, size = dev_q[dev].size(); i < size; ++i)
		waiting[dev_q[dev][i]] = 0;

	dev_q[dev].clear();
	--current_quantum;
	scheduler();
	return tasks;
}

void schedule_new_thread(unsigned next, unsigned pos, int wait) {
	if (debug)
		fprintf(stderr, "next %u, current %u\n", next, current_thread);
	current_pos[prio[next]] = pos;
	current_quantum = quantum;
	if (next != current_thread) {
		unsigned p = current_thread; // Valoare temporara pentru wait
		current_thread = next;

		if (debug)
			fprintf(stderr, "Waking up sem %p\n", sem[next]);

		DIE(sem_post(sem[next]) == -1, "post");
		if (wait)
			DIE(sem_wait(sem[p]) == -1, "wait");
	}
}

void scheduler(int wait) {
	register unsigned priority = prio[current_thread];

	//Preemptie
	for (unsigned int i = SO_MAX_PRIO; i > priority; --i) {
		for (unsigned int j = current_pos[i], size = tree[i].size(); j < size;
				++j)
			if (!waiting[tree[i][j]]) {
				schedule_new_thread(tree[i][j], j, wait);
				return;
			}
		for (unsigned int j = 0; j < current_pos[i]; ++j)
			if (!waiting[tree[i][j]]) {
				schedule_new_thread(tree[i][j], j, wait);
				return;
			}

	}

	if (current_quantum == 0 || waiting[current_thread]) {
		//Yielding
		current_pos[priority]++;
		if (current_pos[priority] >= tree[priority].size())
			current_pos[priority] = 0;

		for (int i = priority; i >= 0; --i) {
			for (unsigned int j = current_pos[i], size = tree[i].size();
					j < size; ++j)
				if (!waiting[tree[i][j]]) {
					schedule_new_thread(tree[i][j], j, wait);
					return;
				}
			for (unsigned int j = 0; j < current_pos[i]; ++j)
				if (!waiting[tree[i][j]]) {
					schedule_new_thread(tree[i][j], j, wait);
					return;
				}
		}
	}

}

int so_wait(unsigned dev) {
	if (dev >= io_max)
		return -1;
	waiting[current_thread] = 1;

	//Adaugam ca dependinta pe device-ul respectiv
	dev_q[dev].push_back(current_thread);
	scheduler();
	return 0;
}

void so_exec() {
	--current_quantum;

	if (debug)
		fprintf(stderr, "thread_id: %u qunatum: %u\n", current_thread,
				current_quantum);

	scheduler();
}

void so_end() {
	void* ret;

	//Cleanup
	for (unsigned i = 0; i < threads.size(); ++i)
		pthread_join(threads[i], &ret);
	for (unsigned i = 0; i < sem.size(); ++i) {
		DIE(sem_destroy(sem[i]) == -1, "destroy");
		free(sem[i]);
	}
	sem.clear();
	threads.clear();

	for (unsigned i = 0; i <= SO_MAX_PRIO; ++i)
		tree[i].clear();

	waiting.clear();

	for (unsigned i = 0; i < dev_q.size(); ++i)
		dev_q[i].clear();
	dev_q.clear();
}

