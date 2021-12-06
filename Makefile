all:
	@echo "Targets: kernel, user"

# kernel
obj-m += kernel/kmod.o

kernel: kbuild kinstall

kbuild:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/kernel modules

kclean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/kernel clean

kinstall: build
	sudo insmod kernel/kmod.ko

# user

user: ubuild urun

ubuild:
	gcc user/main.c -o user/main

uclean:
	rm -f user/main

urun:
	sudo user/main Makefile

