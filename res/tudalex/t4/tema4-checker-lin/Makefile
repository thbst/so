all: build
build: libscheduler.so

libscheduler.so: main.cpp
	g++ main.cpp -Wall -g -shared -fPIC -o libscheduler.so

clean:
	rm -f libscheduler.so

zip: archive
archive:
	zip tema4_lin.zip main.cpp Makefile so_scheduler.h Readme.txt