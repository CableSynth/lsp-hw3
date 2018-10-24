ifneq ($(KERNELRELEASE),)
# call from kernel build system

# the next line was removed from the makefile to accommodate renaming the
# source code to scull.c versus scull_main.c
# scull-objs := scull_main.o

obj-m	:= scull.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o *.ko *.mod.c *.order *.symvers

