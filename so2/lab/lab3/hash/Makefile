KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f *~ Module.symvers

deploy:
	cp hashtest.ko ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
