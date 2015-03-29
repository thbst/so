CC=cl
CFLAGS=/W3 /nologo /EHac
OBJ_PARSER=parser.tab.obj parser.yy.obj
OBJ=main.obj utils-win.obj
TARGET=mini-shell.exe

build: $(TARGET)

$(TARGET): $(OBJ) $(OBJ_PARSER)
	$(CC) $(CFLAGS) $(OBJ) $(OBJ_PARSER) /Fe$(TARGET)

.cpp.obj:
    $(CPP) $(CFLAGS) /c $*.cpp

clean:
	 del /Q /F $(OBJ) $(OBJ_PARSER) $(TARGET) *~
