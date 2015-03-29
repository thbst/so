/*
 * lib.cpp
 *
 *  Created on: 10.04.2013
 *      Author: tudalex
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <Windows.h>

#define EXPORTS
#include "mpi.h"

#define MAX_MSG 8192

MPI_Comm mpi_comm_world;


// Citeste parametrii si ii inlocuieste
int MPI_Init(int *argc, char ***argv) {
	int context, rank, size;
	if (mpi_comm_world != NULL)
			return MPI_ERR_OTHER;
	if (sscanf((*argv)[1],"m%d", &context)) {
		sscanf((*argv)[2],"%d", &rank);
		sscanf((*argv)[3],"%d", &size);
		*argc-=4;
		*argv+=4;

		// Allocam structura
		mpi_comm_world = (MPI_Comm) calloc(1, sizeof(struct mpi_comm));
		DIE(mpi_comm_world == NULL, "calloc");
		mpi_comm_world->context = context;
		mpi_comm_world->rank = rank;
		mpi_comm_world->size = size;
		char name[32];
		sprintf(name, "\\\\.\\mailslot\\%d\\%d", context, rank); 
		HANDLE hMS = CreateMailslot(name, MAX_MSG, MAILSLOT_WAIT_FOREVER,NULL);
		DIE(hMS == INVALID_HANDLE_VALUE, "CreateMailslot failed");
		mpi_comm_world->ms = hMS;
		Sleep(50);
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
	CloseHandle(mpi_comm_world->ms);
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
	struct _mpi_message t;
	t.count = count*type_size[datatype];
	memcpy(t.data, buf, t.count);
	t.sender = comm->rank;
	t.tag = tag;
	bool bRet;
	DWORD cbWritten, dwRet;

	//Dechid queue-ul si trimit datele de pe el
	char name[32];
	sprintf(name, "\\\\.\\mailslot\\%d\\%d", comm->context, dest); 
	HANDLE hMailslot = CreateFile(name,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	DIE(hMailslot == INVALID_HANDLE_VALUE, "CreateFile");

	bRet = WriteFile(
		hMailslot,
		&t,
		sizeof(t),
		&cbWritten,
		(LPOVERLAPPED)NULL);
	DIE(bRet == FALSE, "Write file to Mailslot");
	
	/* Close Mailslot */
	dwRet = CloseHandle(hMailslot);
	DIE (dwRet == FALSE, "CloseHandle");

	return MPI_SUCCESS;
}


int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
		      int source, int tag, MPI_Comm comm, MPI_Status *status) {
	struct _mpi_message t;

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
	bool bRet;
	DWORD cbRead;
	bRet = ReadFile(
		comm->ms,
 		&t,
		sizeof(struct _mpi_message),
 		&cbRead,
 		(LPOVERLAPPED)NULL);
	DIE(bRet == FALSE, "ReadFile from Mailslot");

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
