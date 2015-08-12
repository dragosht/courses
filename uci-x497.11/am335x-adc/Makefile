KDIR=/home/dragosh/workspace/beagle/debian/linux-3.8/bb-kernel/KERNEL/

kbuild:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KDIR) M=`pwd`

clean:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf -C $(KDIR) M=`pwd` clean *~

deploy:
	scp am335x-adc.ko root@192.168.7.2://root/
