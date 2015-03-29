CC = cl
CFLAGS = /nologo /W3 /EHsc -D_CRT_SECURE_NO_DEPRECATE

#Aparent variabila $LIB e map-uita catre un folder si aveam efecte dubioase
LIB_NAME = sompi

IMPLIB=$(LIB_NAME).lib
DLL=$(LIB_NAME).dll


build: all

all: $(DLL) mpirun.exe

$(DLL): lib.obj
	link /nologo /dll /out:$(DLL) /implib:$(IMPLIB) $**

lib.cpp: mpi.h mpi_err.h

mpirun.exe: mpirun.obj
	$(CC) /F$@ $**

mpirun.cpp: mpi.h mpi_err.h

.cpp.obj:
    $(CC) $(CFLAGS) /c $*.cpp
clean:
	del $(IMPLIB) $(DLL) lib.obj mpirun.obj mpirun.exe