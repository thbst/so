CFLAGS=/W3 /D_CRT_SECURE_NO_DEPRECATE

build: tema1.exe

tema1.exe: main.obj hashtable.obj hash.obj utils.obj
	cl $(CFLAGS) /Fetema1.exe main.obj hashtable.obj hash.obj utils.obj

main.obj: main.c
	cl $(CFLAGS) /Fomain.obj /c main.c

hash.obj: hash.c
	cl $(CFLAGS)  /Fohash.obj /c hash.c

hashtable.obj: hashtable.c
	cl $(CFLAGS) /Fohashtable.obj /c hashtable.c

utils.obj: utils.c
	cl $(CFLAGS) /Foutils.obj /c utils.c

clean:
	del *.obj *.exe *.out
