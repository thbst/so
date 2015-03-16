CC=gcc
CFLAGS= -Wall -g
OBJECTS = tema1.o util.o hash.o

build: tema1

tema1: $(OBJECTS)
	$(CC) $(CFLAGS) -o tema1 $(OBJECTS)

.PHONY: clean

clean:
	rm -f *.o *~ tema1

zip:
	zip CC334_CristianLupu_SO_tema1.zip tema1.c util.c util.h README Makefile GNUmakefile
