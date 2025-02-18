# Define both kernel modules
obj-m := ti-ads114s0xb.o

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	install -m 644 ti-ads114s0xb.ko /lib/modules/$(shell uname -r)/kernel/drivers/
	depmod -a

insmod:
	sudo insmod ti-ads114s0xb.ko

rmmod:
	sudo rmmod ti-ads114s0xb

