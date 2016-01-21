KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
	make -f Makefile.format

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f Module.symvers Module.markers modules.order *~
	rm -f ../../../../qemu-vm/fsimg/root/minfs.ko
	rm -f ../../../../qemu-vm/fsimg/root/mkfs.minfs
	rm -f ../../../../qemu-vm/test-minfs-0.sh
	rm -f ../../../../qemu-vm/test-minfs-1.sh
	rm -f ../../../../qemu-vm/test-minfs-2.sh
	make -f Makefile.format clean


deploy:
	cp minfs.ko ../../../../qemu-vm/fsimg/root/
	cp mkfs.minfs ../../../../qemu-vm/fsimg/root/
	cp test-minfs-0.sh ../../../../qemu-vm/fsimg/root/
	cp test-minfs-1.sh ../../../../qemu-vm/fsimg/root/
	cp test-minfs-2.sh ../../../../qemu-vm/fsimg/root/
	cd ../../../../qemu-vm/ && make
