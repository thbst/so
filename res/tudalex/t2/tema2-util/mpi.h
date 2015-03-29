#ifndef MPI_H_
#define MPI_H_

#include "mpi_err.h"
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>


#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)




#if defined(_WIN32)

#ifdef EXPORTS
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif

#else

#define DECLSPEC
#endif


struct mpi_comm {
	int context;
	int rank;
	int size;
};
typedef mpi_comm *MPI_Comm;


#define MPI_COMM_WORLD (mpi_comm_world)
#define packet_size (8192-3*sizeof(int))

typedef unsigned char MPI_Datatype;

#define MPI_CHAR	0
#define MPI_INT		1
#define MPI_DOUBLE	2

const int type_size[3] = { 1, 4, 8};

// Structura de status
struct mpi_status {
	int MPI_SOURCE;
	int MPI_TAG;
	int _size;
};

// Structura de mesaj ce va fi transmis
struct _mpi_message {
	int count;
	int sender;
	int tag;
	char data[packet_size];
};


typedef struct _mpi_message mpi_message;

typedef struct mpi_status MPI_Status;

#define MPI_ANY_SOURCE	(0xffffeeee)
#define MPI_ANY_TAG	(0xaaaabbbb)
#define MPI_STATUS_IGNORE ((MPI_Status *)0xabcd1234)


// Am vrut sa rezolv tema in C++, fiind ca initial aveam impresia ca este mai
// complicata
extern "C" {
	extern MPI_Comm mpi_comm_world;
	int DECLSPEC MPI_Init(int *argc, char ***argv); //done
	int DECLSPEC MPI_Initialized(int *flag);
	int DECLSPEC MPI_Comm_size(MPI_Comm comm, int *size);
	int DECLSPEC MPI_Comm_rank(MPI_Comm comm, int *rank);
	int DECLSPEC MPI_Finalize();
	int DECLSPEC MPI_Finalized(int *flag);

	int DECLSPEC MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest,
			int tag, MPI_Comm comm);

	int DECLSPEC MPI_Recv(void *buf, int count, MPI_Datatype datatype,
			int source, int tag, MPI_Comm comm, MPI_Status *status);


	int DECLSPEC MPI_Get_count(MPI_Status *status, MPI_Datatype datatype,
			int *count);
}
#endif
