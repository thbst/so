/*
 * lib.cpp
 *
 *  Created on: 10.04.2013
 *      Author: tudalex
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>

#include "mpi.h"

MPI_Comm mpi_comm_world;

// Functie helper ce deschide o coada
static mqd_t open_queue(int x, bool read) {
	char name[12];
	mqd_t q;
	sprintf(name, "/%d_%d", mpi_comm_world->context, x);
	if (read)
		q = mq_open(name, O_RDONLY);
	else
		q = mq_open(name, O_WRONLY);
	DIE(q == (mqd_t)-1, "Can't open queue");
	return q;
}

// Citeste parametrii si ii inlocuieste
int MPI_Init(int *argc, char ***argv) {
	int context, rank, size;
	if (mpi_comm_world != NULL)
			return MPI_ERR_OTHER;
	if (sscanf((*argv)[0],"m%d", &context)) {
		sscanf((*argv)[1],"%d", &rank);
		sscanf((*argv)[2],"%d", &size);
		*argc-=3;
		*argv+=3;

		// Allocam structura
		mpi_comm_world = (MPI_Comm) calloc(1, sizeof(struct mpi_comm));
		DIE(mpi_comm_world == NULL, "calloc");
		mpi_comm_world->context = context;
		mpi_comm_world->rank = rank;
		mpi_comm_world->size = size;

		return MPI_SUCCESS;
	}
	return MPI_ERR_IO;
}

int MPI_Initialized(int *flag) {
	*flag = mpi_comm_world != NULL;
	return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int *size) {
	// Sanity checks
	if (mpi_comm_world != comm)
		return MPI_ERR_COMM;
	if (mpi_comm_world == NULL || mpi_comm_world == (MPI_Comm)0xFFFFFFFF) {
		fprintf(stderr,"%p",mpi_comm_world);
		return MPI_ERR_OTHER;
	}
	*size = comm->size;
	return MPI_SUCCESS;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank) {
	if (mpi_comm_world != comm)
			return MPI_ERR_COMM;
	if (mpi_comm_world == NULL || mpi_comm_world == (MPI_Comm)0xFFFFFFFF)
				return MPI_ERR_OTHER;
	*rank = comm->rank;
	return MPI_SUCCESS;
}

int MPI_Finalize() {
	if (mpi_comm_world == NULL || mpi_comm_world == (MPI_Comm)0xFFFFFFFF)
			return MPI_ERR_OTHER;
	free(mpi_comm_world);
	mpi_comm_world = (MPI_Comm)0xFFFFFFFF;
	return MPI_SUCCESS;
}

int MPI_Finalized(int *flag) {
	*flag = mpi_comm_world == (MPI_Comm)0xFFFFFFFF;
	return MPI_SUCCESS;
}

int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest,
		      int tag,
		      MPI_Comm comm) {
	// Sanity checks
	if (mpi_comm_world == NULL || mpi_comm_world == (MPI_Comm)0xFFFFFFFF)
		return MPI_ERR_OTHER;
	if (comm != mpi_comm_world)
		return MPI_ERR_COMM;
	if (datatype < 0 || datatype > 2)
		return MPI_ERR_TYPE;
	if (dest < 0 || dest > comm->size)
		return MPI_ERR_RANK;
	int rc;
	struct _mpi_message t;
	t.count = count*type_size[datatype];
	memcpy(t.data, buf, t.count);
	t.sender = comm->rank;
	t.tag = tag;

	//Dechid queue-ul si trimit datele de pe el
	mqd_t q = open_queue(dest, false);
	rc = mq_send(q, (char*)&t, sizeof(mpi_message),	0);
	DIE(rc == -1,"Send");
	mq_close(q);

	return MPI_SUCCESS;
}


int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
		      int source, int tag, MPI_Comm comm, MPI_Status *status) {
	struct _mpi_message t;
	int rc;

	//Sanity checks
	if (mpi_comm_world == NULL || mpi_comm_world == (MPI_Comm)0xFFFFFFFF)
		return MPI_ERR_OTHER;
	if (comm != mpi_comm_world)
		return MPI_ERR_COMM;
	if ((unsigned int)tag != MPI_ANY_TAG)
		return MPI_ERR_TAG;
	if (datatype < 0 || datatype > 2)
		return MPI_ERR_TYPE;
	if ((unsigned int)source != MPI_ANY_SOURCE)
		return MPI_ERR_RANK;


	mqd_t q = open_queue(comm->rank, true);
	rc = mq_receive(q,(char*)&t, sizeof(mpi_message), NULL);
	DIE(rc == -1, "Recieve failed");
	mq_close(q);


	if (status != MPI_STATUS_IGNORE && status!=NULL) {
		status->MPI_TAG = t.tag;
		status->MPI_SOURCE = t.sender;
		status->_size = t.count;
	}
	memcpy(buf, t.data, t.count);

	return MPI_SUCCESS;

}

int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count) {
	if (mpi_comm_world == NULL || mpi_comm_world == (MPI_Comm)0xFFFFFFFF)
			return MPI_ERR_OTHER;
	*count = status->_size/type_size[datatype];
	return MPI_SUCCESS;
}
