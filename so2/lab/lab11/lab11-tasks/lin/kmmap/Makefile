KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
	cd ../test/ && make

clean:
	make -C $(KDIR) M=`pwd` clean
	-rm -f Module.symvers *~
	rm -f ../../../../../qemu-vm/fsimg/root/kmmap.ko
	cd ../test/ && make clean
	rm -f ../../../../../qemu-vm/fsimg/root/mmap-test

deploy:
	cp kmmap.ko ../../../../../qemu-vm/fsimg/root/
	cp ../test/mmap-test ../../../../../qemu-vm/fsimg/root/
	cd ../../../../../qemu-vm/ && make

