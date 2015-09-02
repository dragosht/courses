CFLAGS = -Wall -g -m32 -static

.PHONY: all clean

all: test

deploy:
	cp test ../../../../qemu-vm/fsimg/root/

clean:
	-rm -f *.o *~ test
	rm -f ../../../../qemu-vm/fsimg/root/test


