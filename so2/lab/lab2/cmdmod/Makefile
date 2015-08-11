KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f *~ Module.symvers Module.markers modules.order

deploy:
	cp cmdmod.ko ../../../qemu-vm/fsimg/root/
	cp insert.sh ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
