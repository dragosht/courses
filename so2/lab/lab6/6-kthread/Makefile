KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f ../../../qemu-vm/fsimg/root/kthread.ko

deploy:
	cp kthread.ko ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
