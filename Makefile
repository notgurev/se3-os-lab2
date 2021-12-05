all:
	@echo "Targets: kernel, user"

# kernel
obj-m += kmod.o

kernel: kbuild kinstall

kbuild:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

kclean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

kinstall: build
	sudo insmod kmod.ko

# user

user: ubuild urun

ubuild:
	gcc main.c -o main

uclean:
	rm -f main

urun:
	sudo ./main main.c