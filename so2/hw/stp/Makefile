KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
	cd test && make -f Makefile.checker

clean:
	make -C $(KDIR) M=`pwd` clean
	cd test && make -f Makefile.checker clean
	rm -f ../../qemu-vm/fsimg/root/_checker
	rm -f ../../qemu-vm/fsimg/root/af_stp.ko
	rm -f ../../qemu-vm/fsimg/root/stp_test

deploy:
	cp af_stp.ko ../../qemu-vm/fsimg/root/
	cp test/_checker ../../qemu-vm/fsimg/root/
	cp test/stp_test ../../qemu-vm/fsimg/root/
	cd ../../qemu-vm/ && make


