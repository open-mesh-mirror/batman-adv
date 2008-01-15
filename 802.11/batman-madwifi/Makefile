
obj-m += batman-adv-mad.o

CFLAGS += -I ~/madwifi -I /usr/src/madwifi-svn

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
