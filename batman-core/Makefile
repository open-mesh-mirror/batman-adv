#
# Run with "make KERNELPATH=/path/to/linux-x.y.z" for special kernel path.
# Just typing "make" will use your installed kernel.
#


ifeq ($(wildcard $(KERNELPATH)),)
KERNELPATH = /lib/modules/$(shell uname -r)/build
# sanity check: does KERNELPATH exist?
ifeq ($(shell cd $(KERNELPATH) && pwd),)
$(error $(KERNELPATH) is missing, please set KERNELPATH)
endif
export KERNELPATH
endif

obj-m += batman-adv-core.o
batman-adv-core-objs := batman-adv-main.o batman-adv-proc.o batman-adv-send.o batman-adv-log.o batman-adv-routing.o batman-adv-interface.o batman-adv-device.o batman-adv-ttable.o bitarray.o hash.o ring_buffer.o

all:
	make -C $(KERNELPATH) M=$(PWD) modules

clean:
	make -C $(KERNELPATH) M=$(PWD) clean
