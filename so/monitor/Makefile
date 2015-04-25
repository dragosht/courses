CFLAGS=-Wall -g -ggdb -pipe -pedantic -std=gnu99

all: LibMonitor.so LibRW.so mytest

LibMonitor.so: monitor.o
	gcc -shared monitor.o -o LibMonitor.so -lpthread

LibRW.so: LibMonitor.so rw.o
	gcc -shared rw.o -o LibRW.so LibMonitor.so -lpthread

rw.o: rw.c
	gcc $(CFLAGS) -c -fPIC rw.c

monitor.o: monitor.c
	gcc $(CFLAGS) -c -fPIC monitor.c

mytest: mytest.c
	gcc $(CFAGS) -o mytest mytest.c LibMonitor.so -lpthread

test:
	make -f Makefile.checker build-pre && export LD_LIBRARY_PATH=. && ./Test

clean:
	rm -f *.o mytest *.so
