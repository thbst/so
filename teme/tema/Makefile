CC=gcc
CFLAGS= -Wall -g

build: tema

tema: tema.o util.o
	$(CC) $(CFLAGS) -o tema tema.o util.o

.PHONY: clean

clean:
	rm -f *.o *~ tema

zip:
	zip Grupa_PrenumeNume_SO_tema.zip tema.c util.c util.h README Makefile GNUmakefile
