build: vmsim.so
	
vmsim.so: vmsim.o common_lin.o
	g++ -shared vmsim.o common_lin.o -o libvmsim.so

vmsim.o: vmsim.cpp
	g++ -Wall -fPIC -c vmsim.cpp -o vmsim.o
common_lin.o: common_lin.c
	g++ -Wall -fPIC -c common_lin.c -o common_lin.o

clean:
	rm libvmsim.so vmsim.o common_lin.o
