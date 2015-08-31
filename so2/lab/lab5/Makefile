KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	-rm -f *~ Module.symvers Module.markers modules.order
	rm -f ../../qemu-vm/fsimg/root/so2_kbd.ko
	rm -f ../../qemu-vm/fsimg/root/mknodes.sh

deploy:
	cp so2_kbd.ko ../../qemu-vm/fsimg/root/
	cp mknodes.sh ../../qemu-vm/fsimg/root/
	cd ../../qemu-vm/ && make

