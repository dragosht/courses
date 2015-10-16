KDIR=/usr/src/linux-so2

all: kbuild

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean

deploy:
	cp ssr.ko ../../qemu-vm/fsimg/root/
	cd ../../qemu-vm/ && make
