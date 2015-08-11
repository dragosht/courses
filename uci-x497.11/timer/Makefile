ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)


modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *.cmd *.ko *.mod.c .tmp_versions *~ *.symvers

.PHONY: modules modules_install clean

else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m:=timer_driver.o

endif

