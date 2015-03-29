CC = gcc
CFLAGS = -Wall -g

all: build

build: tema0.o hash.o operatii.o executaComenzi.o
	$(CC) $(CFLAGS) -o tema0 hash.o tema0.o operatii.o executaComenzi.o

tema0.o: tema0.c
#	gcc -c tema0.c
hash.o: hash.c
#	gcc -c hash.c	
operatii.o: operatii.c
executaComenzi.o: executaComenzi.c

clean:
	rm -r *.o tema0
	
