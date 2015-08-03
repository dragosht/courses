#KDIR = /lib/modules/`uname -r`/build
KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean *~

deploy:
	cp probes.ko ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
