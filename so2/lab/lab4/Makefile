all: deploy

deploy:
	cd kernel && make
	cd user && make
	cp kernel/so2_cdev.ko ../../qemu-vm/fsimg/root/
	cp kernel/mknodes.sh ../../qemu-vm/fsimg/root/
	cp user/so2_cdev_test ../../qemu-vm/fsimg/root
	cd ../../qemu-vm/ && make

clean:
	cd kernel && make clean
	cd user && make clean
	rm -f ../../qemu-vm/fsimg/root/so2_cdev.ko
	rm -f ../../qemu-vm/fsimg/root/mknodes.sh
	rm -f ../../qemu-vm/fsimg/root/so2_cdev_test

