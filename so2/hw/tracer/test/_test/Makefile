#CPPFLAGS = -DDEBUG -DLOG_LEVEL=LOG_DEBUG
CFLAGS = -Wall -g -static -m32 -DDEBUG

.PHONY: all clean

all: tracer_test.o

stp_test.o: tracer_test.c tracer_test.h tracer.h test.h util.h debug.h

clean:
	-rm -f *~ *.o
