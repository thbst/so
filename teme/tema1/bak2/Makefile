CC = cl
CFLAGS= /nologo /W3 -D CRT_SECURE_NO_DEPRECATE
PGR = tema1

all: build
build: hash.c tema1.c util.c
	$(CC) $(CFLAGS) /Fe$(PGR).exe hash.c tema1.c util.c

clean:
	del $(PGR).exe *.obj

