
obj-m += batman-adv-core.o
batman-adv-core-objs := batman-adv-main.o batman-adv-proc.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
