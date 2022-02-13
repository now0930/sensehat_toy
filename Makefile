obj-m += sensehat.o 

.PHONY: build clean load unload

build:
#make -C /lib/modules/$(shell uname -r)/build modules M=$(PWD)
	make -C /lib/modules/`uname -r`/build M=`pwd` modules

clean:
	make -C /lib/modules/$(shell uname -r)/build clean M=$(PWD)

load:
	sudo insmod sensehat.ko
unload:
	-sudo rmmod sensehat
