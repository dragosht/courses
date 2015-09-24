KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f *~ Module.symvers
	rm -f ../../qemu-vm/fsimg/root/uart16550.ko

deploy:
	cp uart16550.ko ../../qemu-vm/fsimg/root/
	cp mknodes.sh ../../qemu-vm/fsimg/root
	cd ../../qemu-vm/ && make

check:
	#$(KDIR)/scripts/checkpatch.pl -f -no-tree list.c
	#cppcheck list.c
	make -C $(KDIR) M=`pwd` C=2 `pwd`
