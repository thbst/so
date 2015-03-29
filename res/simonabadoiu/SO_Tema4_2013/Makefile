build: libscheduler.so
	
libscheduler.so: so_scheduler.o queue.o
	gcc -shared so_scheduler.o queue.o -o libscheduler.so -lpthread

so_scheduler.o: so_scheduler.c
	gcc -Wall -fPIC -c so_scheduler.c -o so_scheduler.o -lpthread
	
queue.o: queue.c
	gcc -Wall -fPIC -c queue.c -o queue.o 
	
clean:
	rm libscheduler.so so_scheduler.o queue.o
