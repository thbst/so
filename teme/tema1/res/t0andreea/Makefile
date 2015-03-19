CFLAGS = /nologo /W3
OBJ_LIST = hash.obj operatii.obj executaComenzi.obj

all: build
build: tema0.exe
tema0.exe: tema0.obj $(OBJ_LIST)
	$(CPP) $(CFLAGS) /Fe$@ $**

tema0.obj: tema0.c
	cl /Fo$@ /c tema0.c
hash.obj: hash.c
	cl /Fo$@ /c hash.c
operatii.obj: operatii.c
	cl /Fo$@ /c operatii.c
executaComenzi.obj: executaComenzi.c
	cl /Fo$@ /c executaComenzi.c
clean:
	del *.obj tema0.exe
