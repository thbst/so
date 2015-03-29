CC = gcc
CFLAGS = -Wall -Wextra 

all: build

.PHONY: build clean

build: 
	$(CC) $(CFLAGS) -o tema1 main.c hashtable.c utils.c hash.c
	
clean:
	rm -f tema1.o tema1 *.out
