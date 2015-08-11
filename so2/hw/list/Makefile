KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f *~ Module.symvers

deploy:
	cp list.ko ../../qemu-vm/fsimg/root/
	cp list-checker.sh ../../qemu-vm/fsimg/root/
	cd ../../qemu-vm/ && make

check:
	#$(KDIR)/scripts/checkpatch.pl -f -no-tree list.c
	#cppcheck list.c
	make -C $(KDIR) M=`pwd` C=2 `pwd`
