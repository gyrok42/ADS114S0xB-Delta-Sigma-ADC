# Define both kernel modules
obj-m := ads114s0xb.o ads114s0xb-mock.o

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
