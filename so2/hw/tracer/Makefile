KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f ../../qemu-vm/fsimg/root/tracer.ko
	cd ./test && make -f Makefile.checker clean
	cd ./trtest && make clean
	rm -f *~ Module.symvers

deploy:
	cp tracer.ko ../../qemu-vm/fsimg/root/
	cd trtest && make deploy
	cd ./test && make -f Makefile.checker deploy
	cd ../../qemu-vm/ && make
