CC=gcc
CFLAGS=-c -Wall -std=c89 -g
LDFLAGS=
SOURCES=hash.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=tema0

all: build

build: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o tema0
