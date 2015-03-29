CC = gcc
CFLAGS = -Wall -g
MAIN = tema1
OBJECTS = tema1.o util.o hash.o

build: $(MAIN)

$(MAIN): hash.c tema1.c util.c
	$(CC) $(CFLAGS) $^ -o $(MAIN)
.PHONY: clean
clean:
	rm -f *~ $(MAIN) *.o *.zip

run: 
	make clean; make build; ./tema1 10 < hash1.in
zip:
	zip Grupa_NumePrenume_SO_tema1.zip tema1.c util.c util.h hash.c hash.h README Makefile GNUmakefile ENUNT
