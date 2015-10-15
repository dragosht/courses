KDIR = /usr/src/linux-so2

all: kbuild ram-disk-test

ram-disk-test: ram-disk-test.c
	gcc -o ram-disk-test ram-disk-test.c -m32 -Wall -g -ggdb -static

kbuild:
	make -C $(KDIR) M=$(shell pwd)

clean:
	make -C $(KDIR) M=$(shell pwd) clean
	rm -f ram-disk-test *.o
	rm -f ../../../qemu-vm/fsimg/root/ram-disk.ko
	rm -f ../../../qemu-vm/fsimg/root/ram-disk-test
	rm -f ../../../qemu-vm/fsimg/root/mknodes.sh
	-rm -f *~

deploy:
	cp ram-disk.ko ../../../qemu-vm/fsimg/root/
	cp ram-disk-test ../../../qemu-vm/fsimg/root/
	cp mknodes.sh ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
