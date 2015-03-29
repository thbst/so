build: mpirun libsompi.so
	
mpirun: mpi.c
	gcc -Wall -g mpi.c -o mpirun -lrt
libsompi.so: mpi.o
	gcc -shared mpi.o -o libsompi.so -lrt

mpi.o: mpi.c
	gcc -Wall -fPIC -c mpi.c -o mpi.o -lrt

clean:
	rm mpirun libsompi.so mpi.o
