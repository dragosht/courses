KDIR = /usr/src/linux-so2

all: kbuild testbuild

kbuild:
	make -C $(KDIR) M=`pwd`

testbuild:
	cd test && make -f Makefile.checker

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f *~ Module.symvers
	cd test && make -f Makefile.checker clean
	rm -f ../../qemu-vm/fsimg/root/uart16550.ko
	rm -f ../../qemu-vm/fsimg/root/solutie.ko
	rm -f ../../qemu-vm/fsimg/root/test
	rm -f ../../qemu-vm/fsimg/root/_checker

deploy:
	cp uart16550.ko ../../qemu-vm/fsimg/root/
	cp mknodes.sh ../../qemu-vm/fsimg/root/
	cp test/test ../../qemu-vm/fsimg/root/
	cp test/_checker ../../qemu-vm/fsimg/root/
	cp test/_test/solutie.ko ../../qemu-vm/fsimg/root/
	cd ../../qemu-vm/ && make

check:
	#$(KDIR)/scripts/checkpatch.pl -f -no-tree list.c
	#cppcheck list.c
	make -C $(KDIR) M=`pwd` C=2 `pwd`
