CC = gcc
FLAGS = -Wall -g
LIST = main.o hashtable.o linkedlist.o hash.o

build: $(LIST)
	$(CC) $(FLAGS) $(LIST) -o tema1

hash.o: hash.h hash.c
	$(CC) $(FLAGS) -c -o hash.o hash.c

linkedlist.o: linkedlist.h linkedlist.c utils.h
	$(CC) $(FLAGS) -c -o linkedlist.o linkedlist.c

hashtable.o: hashtable.h hashtable.c
	$(CC) $(FLAGS) -c -o hashtable.o hashtable.c
	
main.o: main.c
	$(CC) $(FLAGS) -c -o main.o main.c
clean:
	rm tema1 *.o