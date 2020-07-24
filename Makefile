obj-m:=ram_block.o

kdir = /lib/modules/$(shell uname -r)/build

all:
	make -C $(kdir) M=$(shell pwd) modules
	gcc test_app.c
clean:
	make -C $(kdir) M=$(shell pwd) clean
	rm -rf a.out
