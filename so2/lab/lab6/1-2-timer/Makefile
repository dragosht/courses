KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f ../../../qemu-vm/fsimg/root/timer.ko

deploy:
	cp timer.ko ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
