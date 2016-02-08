KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
	make -f Makefile.test

clean:
	make -C $(KDIR) M=`pwd` clean
	make -f Makefile.test clean
	rm -f ../../../../qemu-vm/fsimg/root/filter.ko
	rm -f ../../../../qemu-vm/fsimg/root/test
	rm -f ../../../../qemu-vm/fsimg/root/test-1.sh
	rm -f ../../../../qemu-vm/fsimg/root/test-2.sh

deploy:
	cp filter.ko ../../../../qemu-vm/fsimg/root/
	cp test ../../../../qemu-vm/fsimg/root/
	cp test-1.sh ../../../../qemu-vm/fsimg/root/
	cp test-2.sh ../../../../qemu-vm/fsimg/root/
	cd ../../../../qemu-vm/ && make

