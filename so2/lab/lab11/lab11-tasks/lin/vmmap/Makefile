KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
	cd ../test/ && make

clean:
	make -C $(KDIR) M=`pwd` clean
	-rm -f Module.symvers *~
	rm -f ../../../../../qemu-vm/fsimg/root/vmmap.ko
	cd ../test/ && make clean
	rm -f ../../../../../qemu-vm/fsimg/root/mmap-test

deploy:
	cp vmmap.ko ../../../../../qemu-vm/fsimg/root/
	cp ../test/mmap-test ../../../../../qemu-vm/fsimg/root/
	cd ../../../../../qemu-vm/ && make

