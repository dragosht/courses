KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f ../../../../qemu-vm/fsimg/root/deferred.ko
	rm -f ../../../../qemu-vm/fsimg/root/makenode

deploy:
	cp deferred.ko ../../../../qemu-vm/fsimg/root/
	cp makenode ../../../../qemu-vm/fsimg/root/
