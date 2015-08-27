CFLAGS=-Wall -g -ggdb -pedantic -m32 -static

all: test

test: test.c

clean:
	rm -f test *~
	rm -f ../../../../qemu-vm/fsimg/root/test

deploy:
	cp test ../../../../qemu-vm/fsimg/root/
