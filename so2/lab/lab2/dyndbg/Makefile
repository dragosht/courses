KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`
 
clean:
	make -C $(KDIR) M=`pwd` clean

deploy:
	cp dyndbg.ko ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
