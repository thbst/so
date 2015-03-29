CC = cl
CFLAGS = /nologo /W3 /EHsc -D_CRT_SECURE_NO_DEPRECATE

#Aparent variabila $LIB e map-uita catre un folder si aveam efecte dubioase
LIB_NAME = libscheduler

IMPLIB=$(LIB_NAME).lib
DLL=$(LIB_NAME).dll

build: all

all: $(DLL)

$(DLL): main.obj
	link /nologo /dll /out:$(DLL) /implib:$(IMPLIB) $**


.cpp.obj:
    $(CC) $(CFLAGS) /c $*.cpp
clean:
	del $(IMPLIB) $(DLL) main.obj