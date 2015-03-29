CC = cl
FLAGS = /W3 -D_CRT_SECURE_NO_DEPRECATE
OBJ_LIST = hash.obj linkedlist.obj hashtable.obj main.obj

build: tema1.exe

tema1.exe: $(OBJ_LIST)
	$(CC) $(FLAGS) /Fetema1.exe $(OBJ_LIST)

hash.obj: hash.h hash.c
	$(CC) $(FLAGS) /c hash.c

linkedlist.obj: linkedlist.h linkedlist.c utils.h
	$(CC) $(FLAGS) /c linkedlist.c

hashtable.obj: hashtable.h hashtable.c
	$(CC) $(FLAGS) /c hashtable.c

main.obj: main.c
	$(CC) $(FLAGS) /c main.c

clean:
	del /Q /F *.obj tema1.exe