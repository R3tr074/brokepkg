obj-m := brokepkg.o
CC := gcc -Wall
BDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(BDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(BDIR) M=$(PWD) clean