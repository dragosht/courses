KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
	gcc -m32 -static -o test test.c -Wall -g -pipe -pedantic

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f ../../../qemu-vm/fsimg/root/af_testp.ko
	rm -f ../../../qemu-vm/fsimg/root/test test

deploy:
	cp af_testp.ko ../../../qemu-vm/fsimg/root/
	cp test ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make


