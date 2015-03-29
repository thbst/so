#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "mpi.h"

#define MQ_MAXMSG			10
#define MAX_IPC_NAME		128
#define MQ_MSGSIZE			8192
#define MSG_MAX_LEN			2048

struct message_t {
	char buf[8180];
	int count;
	int src;
	int tag;
};

struct mpi_comm *mpi_comm_world;
static int flag_init = 0, flag_final = 0;
static int no_procs, id;

int DECLSPEC MPI_Init(int *argc, char ***argv)
{
	if (flag_init == 1) {
		return MPI_ERR_OTHER;
	}
	else {
		flag_init = 1;
	}
	
	/* init the number of processes and the id of this process */
	no_procs = atoi((*argv)[1]);
	id = atoi((*argv)[2]);
	
	return MPI_SUCCESS;
}

int DECLSPEC MPI_Initialized(int *flag)
{
	*flag = flag_init;
	return MPI_SUCCESS;
}

int DECLSPEC MPI_Comm_size(MPI_Comm comm, int *size)
{
	if (flag_init == 0 || flag_final == 1) {
		return MPI_ERR_OTHER;
	}
	
	if (comm != mpi_comm_world) {
		return MPI_ERR_COMM;
	}
	
	*size = no_procs;
	
	return MPI_SUCCESS;
}

int DECLSPEC MPI_Comm_rank(MPI_Comm comm, int *rank)
{
	if (flag_init == 0 || flag_final == 1) {
		return MPI_ERR_OTHER;
	}
	
	if (comm != mpi_comm_world) {
		return MPI_ERR_COMM;
	}
	
	*rank = id;
	
	return MPI_SUCCESS;
}

int DECLSPEC MPI_Finalize()
{
	if (flag_final == 1) {
		return MPI_ERR_OTHER;
	}
	
	if (flag_init == 1) {
		flag_final = 1;
	}
	else {
		return MPI_ERR_IO;
	}
	
	return MPI_SUCCESS;
}

int DECLSPEC MPI_Finalized(int *flag)
{
	*flag = flag_final;
	return MPI_SUCCESS;
}

int DECLSPEC MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest,
		      int tag, MPI_Comm comm)
{
	int rc, size;
	mqd_t m;
	char name[MAX_IPC_NAME];
	struct message_t msg;
	
	/*sanity checks */
	if (flag_init == 0 || flag_final == 1) {
		return MPI_ERR_OTHER;
	}
	
	if (comm != mpi_comm_world) {
		return MPI_ERR_COMM;
	}
	
	if (datatype != MPI_CHAR && datatype != MPI_INT && datatype != MPI_DOUBLE) {
		return MPI_ERR_TYPE;
	}
	
	/* determine the type of the data we have to send */
	switch(datatype) {
		case MPI_INT:
			size = sizeof(int);
			break;
		case MPI_CHAR:
			size = sizeof(char);
			break;
		case MPI_DOUBLE:
			size = sizeof(double);
			break;
		default:
			break;
	}
	
	/* open the queue where we have to send the message */
	snprintf(name, MAX_IPC_NAME, "/queue%i", dest);
	m = mq_open(name, O_WRONLY);
	if (m == (mqd_t)-1)
		return MPI_ERR_IO;
	
	/* send the message */
	memcpy(msg.buf,(char*)buf, count*size);
	msg.count = count;
	msg.src = id;
	msg.tag = tag;
	
	rc = mq_send(m, (const char*)&msg, sizeof(struct message_t), 10);
	if (rc == -1) {
		return MPI_ERR_IO;
	}
	
	/*detach */
	rc = mq_close(m);
	if (rc < 0)
		return MPI_ERR_IO;
	
	return MPI_SUCCESS;
}

int DECLSPEC MPI_Recv(void *buf, int count, MPI_Datatype datatype,
		      int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	int rc, size, aux_count;
	mqd_t m;
	char name[MAX_IPC_NAME];
	struct message_t msg;
	
	/* sanity checks */
	if (flag_init == 0 || flag_final == 1) {
		return MPI_ERR_OTHER;
	}
	
	if (source != MPI_ANY_SOURCE) {
		return MPI_ERR_RANK;
	}
	
	if (tag != MPI_ANY_TAG) {
		return MPI_ERR_TAG;
	}
	
	if (comm != mpi_comm_world) {
		return MPI_ERR_COMM;
	}
	
	if (datatype != MPI_CHAR && datatype != MPI_INT && datatype != MPI_DOUBLE) {
		return MPI_ERR_TYPE;
	}
	
	/* determine the type of the received data */
	switch(datatype) {
		case MPI_INT:
			size = sizeof(int);
			break;
		case MPI_CHAR:
			size = sizeof(char);
			break;
		case MPI_DOUBLE:
			size = sizeof(double);
			break;
		default:
			break;
	}
	
	/* open the queue for receiving */
	snprintf(name, MAX_IPC_NAME, "/queue%i", id);
	m = mq_open(name, O_RDONLY);
	if (m == (mqd_t)-1)
		return MPI_ERR_IO;
	
	rc = mq_receive(m, (char*)&msg, sizeof(struct message_t), NULL);
	if (rc < 0) {
		return MPI_ERR_IO;
	}
	
	/* status update */
	aux_count = (msg.count<=count ? msg.count:count);
	if (status != MPI_STATUS_IGNORE) {
		status->_size = aux_count;
		status->MPI_SOURCE = msg.src;
		status->MPI_TAG = msg.tag;
	}
	
	/* buf update */
	memcpy(buf, msg.buf, aux_count * size);

	/* detach */
	rc = mq_close(m);
	if (rc < 0)
		return MPI_ERR_IO;
	
	return MPI_SUCCESS;
}


int DECLSPEC MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count)
{
	if (flag_init == 0 || flag_final == 1) {
		return MPI_ERR_OTHER;
	}
	
	if (datatype != MPI_CHAR && datatype != MPI_INT && datatype != MPI_DOUBLE) {
		return MPI_ERR_TYPE;
	}
	
	*count = status->_size;
	
	return MPI_SUCCESS;
}


int main(int argc, char* argv[])
{
	int no_proc, i, status, rc;
	pid_t* procs;
	char *executable_name, *id;
	
	char queue_name[MAX_IPC_NAME];
	mqd_t *m;
	struct mq_attr attr;
	
	if (argc < 4)
		return MPI_ERR_IO;
		
	if (strcmp(argv[1],"-np")) {
		return MPI_ERR_IO;
	}
	
	no_proc = atoi(argv[2]);
	
	/* create queues */
	m = malloc(no_proc * sizeof(mqd_t));
	if (!m) {
		return MPI_ERR_IO;
	}
	
	attr.mq_flags = 0;
	attr.mq_maxmsg = MQ_MAXMSG;
	attr.mq_msgsize = sizeof(struct message_t);
	attr.mq_curmsgs = 0;
	
	for (i = 0; i < no_proc; i++) {
		snprintf(queue_name, MAX_IPC_NAME, "/queue%i", i);
		
		m[i] = mq_open(queue_name, O_CREAT | O_RDWR, 0666, &attr);
		if (m[i] == (mqd_t) - 1) {
			return MPI_ERR_IO;
		}
	}
	
	/* create processes */
	executable_name = malloc(100*sizeof(char));
	snprintf(executable_name, 100, "./%s", argv[3]);
	
	procs = malloc(no_proc*sizeof(pid_t));
	
	for (i = 0; i < no_proc; i++) {
		procs[i] = fork();
		
		id = malloc(10*sizeof(char));
		snprintf(id, 10, "%i", i);
		const char *args[] = {executable_name, argv[2], id, NULL};
		
		switch(procs[i]) {
			case -1:
				return MPI_ERR_IO;
				break;
			case 0:
				execvp(argv[3], (char* const*)args);
				exit(EXIT_FAILURE);
				break;
			default:
				break;
		}
		
		free(id);
	}
	
	free(executable_name);
	
	/* wait for every process to finish */
	for (i = 0; i < no_proc; i++) {
		waitpid(procs[i], &status, 0);
	}
	
	/* destroy all the queues */
	for (i = 0; i < no_proc; i++) {
		snprintf(queue_name, MAX_IPC_NAME, "/queue%i", i);

		rc = mq_close(m[i]);
		if (rc < 0) {
			return MPI_ERR_IO;
		}
		
		rc = mq_unlink(queue_name);
		if (rc < 0) {
			return MPI_ERR_IO;
		}
	}
	
	free(m);
	
	return MPI_SUCCESS;
}
