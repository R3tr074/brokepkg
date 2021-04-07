obj-m := brokepkg.o
brokepkg-objs := src/main.o src/hooks.o src/backdoor.o
CC := gcc
KDIR := /lib/modules/$(shell uname -r)/build
ccflags-y += -I$(PWD)/include -Wall

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean