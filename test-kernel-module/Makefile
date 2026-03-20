obj-m += fls.o

KERNEL_SRC := /home/user/65535/workspace/oss/linux/kernel/linux_mainline/build/vng/x86_64
PWD:= $(shell pwd)

all:
	make -C $(KERNEL_SRC) M=$(PWD) modules
clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean
