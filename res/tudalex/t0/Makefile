CC = cl
CFLAGS = /W3 /nologo -D_CRT_SECURE_NO_DEPRECATE
EXE = tema0.exe

all: build

build: main.obj hash.obj
	$(CC) $(CFLAGS) /Fe$(EXE) $**

main.obj: main.c
	$(CC) -c $(CFLAGS) main.c

hash.obj: hash.h hash.c
	$(CC) -c $(CFLAGS) hash.c

.PHONY: clean

clean:
	del *.obj
	del $(EXE)
