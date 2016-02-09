#
# PSO Memory Mapping Lab (#11)
#
# Exercise #1, #2: memory mapping between user-space and kernel-space
#
# test Makefile
#

CFLAGS = -Wall -g -static -m32

all: mmap-test

mmap-test: mmap-test.c

run:
	./mmap-test

.PHONY: clean

clean:
	-rm -f mmap-test *.o *~
