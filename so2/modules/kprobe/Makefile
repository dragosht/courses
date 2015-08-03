#KDIR=/lib/modules/`uname -r`/build
KDIR=/usr/src/linux-so2
VMDIR=~/workspace/qemu-vm
FSDIR=$(VMDIR)/fsimg/root

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean

deploy:
	cp kprobe.ko $(FSDIR)
	cd $(VMDIR) && make
