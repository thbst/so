CC=g++
all: build
build: lib
lib: main.cpp
	g++ -Wall -g -fPIC -shared -o libvmsim.so main.cpp
clean:
	rm libvmsim.so
zip:
	rm tema3_lin.zip
	zip tema3_lin.zip main.cpp vmsim.h Readme Makefile