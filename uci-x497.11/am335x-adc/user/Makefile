CC=arm-linux-gnueabihf-gcc
CFLAGS=-Wall -g -ggdb -pipe -pedantic -std=gnu99

all: adctest

adctest: adctest.c

clean:
	rm -f adctest *~

deploy:
	scp adctest root@192.168.7.2://root/

