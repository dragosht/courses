KDIR=/usr/src/linux-so2

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	rm -f *~ Module.symvers
	rm -f ../../../../qemu-vm/fsimg/root/udp_sock.ko
	rm -f ../../../../qemu-vm/fsimg/root/test-5.sh
	rm -f nc-static


deploy:
	cp udp_sock.ko ../../../../qemu-vm/fsimg/root/
	cp test-5.sh ../../../../qemu-vm/fsimg/root/
	wget http://www.stearns.org/nc/nc-static
	chmod a+x nc-static
	cp nc-static ../../../../qemu-vm/fsimg/root/
	cd ../../../../qemu-vm/ && make
