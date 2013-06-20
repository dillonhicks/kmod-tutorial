MODULENAME=blankslate

obj-m += $(MODULENAME).o



module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -fv *~

install:
	sudo insmod $(MODULENAME).ko
	sudo chmod 666 /dev/$(MODULENAME)

uninstall:
	sudo rmmod $(MODULENAME)

reinstall: uninstall clean module install

