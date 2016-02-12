KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
	make -f Makefile.checker

clean:
	make -C $(KDIR) M=`pwd` clean
	make -f Makefile.checker clean

deploy:
	cp af_stp.ko ../../qemu-vm/fsimg/root/
	cp _checker ../../qemu-vm/fsimg/root/
	cp stp_test ../../qemu-vm/fsimg/root/
	cd ../../qemu-vm/ && make


