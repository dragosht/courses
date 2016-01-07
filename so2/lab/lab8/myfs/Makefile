KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f Module.symvers Module.markers modules.order *~
	rm -f ../../../qemu-vm/fsimg/root/myfs.ko
	rm -f ../../../qemu-vm/fsimg/root/mount.sh
	rm -f ../../../qemu-vm/fsimg/root/test-myfs.sh

deploy:
	cp myfs.ko ../../../qemu-vm/fsimg/root/
	cp test-myfs.sh ../../../qemu-vm/fsimg/root/
	cp mount.sh ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
