CC=g++
all: build
build: mpirun lib

mpirun: mpirun.c mpi.h mpi_err.h
	g++ -Wall -g -o mpirun mpirun.c -lrt
lib: lib.cpp
	g++ -Wall -g -fPIC -shared -o libsompi.so lib.cpp -lrt
clean:
	rm mpirun libsompi.so