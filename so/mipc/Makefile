CFLAGS=-Wall -g -ggdb -pipe -pedantic -std=gnu99

all: client server

client: client.c
	gcc $(CFLAGS) -o client client.c -lrt -lpthread

server: server.c
	gcc $(CFLAGS) -o server server.c -lrt -lpthread

clean:
	rm -f client server *.o *~

