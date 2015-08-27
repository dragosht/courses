KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f ../../../../qemu-vm/fsimg/root/trtest.ko
	rm -f ../../../../qemu-vm/fsimg/root/prep.sh

deploy:
	cp trtest.ko ../../../../qemu-vm/fsimg/root/
	cp prep.sh ../../../../qemu-vm/fsimg/root/
