CC = gcc
CFLAGS = -Wall -g

all:build

build: tema0

tema0: HashTable.o hash.o main.o
	$(CC) -o $@ $^ 

main.o: Main.c
	$(CC) $(CFLAGS) -c $^ -o $@

HashTable.o: HashTable.c
	$(CC) $(CFLAGS) -c $^ -o $@

hash.o:	hash.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf *.o tema0
