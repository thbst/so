#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mpi_err.h"
#include "mpi.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <assert.h>

int main(int argc, char *argv[]) {
	int size, i, status, pid, context;
	char name[12];
	mqd_t *mq;
	if (argc < 3) {
		fprintf(stderr, "Usage: mpirun -np N EXE\n");
	}

	sscanf(argv[2], "%d", &size);
	context = rand()%1000;
	mq =(mqd_t*) calloc(size, sizeof(mqd_t));

	// Structura cu proprietatiile cozii
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 1;
	attr.mq_msgsize = 8192;
	attr.mq_curmsgs = 0;

	// Creez cozile
	for (i = 0; i < size; ++i) {
		sprintf(name,"/%d_%d",context, i);

		mq[i] = mq_open(name, O_CREAT|O_RDONLY, 0644, &attr);
		DIE(mq[i] == (mqd_t)-1, "Can't open queue");
		mq_close(mq[i]);
	}

	// Pornesc procesele
	for (i = 0; i < size; ++i) {
		pid = fork();
		if (pid == 0) {
			sprintf(argv[1], "%d", i);
			sprintf(argv[0], "m%d", context);
			execvp(argv[3], argv);
			exit(-1);
		}
	}

	// Astept dupa ele si afisez return code-urile sau semnalele
	for (i = 0; i < size; ++i) {
		pid = wait(&status);
		if (WIFSIGNALED(status))
			fprintf(stderr, "Process %d was killed by signal %d\n",
				pid, WTERMSIG(status));
		if (WIFEXITED(status))
			fprintf(stderr, "Process %d terminated with exit status %d\n",
					pid, WEXITSTATUS(status));
	}

	// Sterg cozile
	for (i = 0; i < size; ++i) {
		sprintf(name,"/%d_%d", context, i);
		mq_unlink(name);
	}
	free(mq);

	return 0;
}
