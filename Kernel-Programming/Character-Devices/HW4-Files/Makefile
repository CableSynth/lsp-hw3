ifneq ($(KERNELRELEASE),)
# call from kernel build system

KCFLAGS="-Wno-format -Wno-declaration-after-statement"

hw4mod-objs := hw4_mod.o pwd_vault.o

obj-m	:= hw4mod.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o *.ko *.mod.c *.order *.symvers

