KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	cd ./test && make -f Makefile.checker clean
	rm -f *~ Module.symvers

deploy:
	cp tracer.ko ../../qemu-vm/fsimg/root/
	cd ./test && make -f Makefile.checker
	cp ./test/_checker ../../qemu-vm/fsimg/root/
	cp ./test/tracer_helper.ko ../../qemu-vm/fsimg/root
	cp ./test/tracer_test ../../qemu-vm/fsimg/root
	cd ../../qemu-vm/ && make
