KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f ../../../../qemu-vm/fsimg/root/tcp_sock.ko
	rm -f ../../../../qemu-vm/fsimg/root/test-3.sh
	rm -f ../../../../qemu-vm/fsimg/root/test-4.sh

deploy:
	cp tcp_sock.ko ../../../../qemu-vm/fsimg/root/
	cp test-3.sh ../../../../qemu-vm/fsimg/root/
	cp test-4.sh ../../../../qemu-vm/fsimg/root/
	cd ../../../../qemu-vm/ && make

