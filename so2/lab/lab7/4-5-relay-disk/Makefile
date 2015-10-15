KDIR = /usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=$(shell pwd)

clean:
	make -C $(KDIR) M=$(shell pwd) clean
	rm -f ../../../qemu-vm/fsimg/root/relay-disk.ko
	rm -f ../../../qemu-vm/fsimg/root/test-relay-disk
	-rm -f *~

deploy:
	cp relay-disk.ko ../../../qemu-vm/fsimg/root/
	cp test-relay-disk ../../../qemu-vm/fsimg/root/
	cd ../../../qemu-vm/ && make
