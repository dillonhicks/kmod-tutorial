MODULENAME=tabularasa

obj-m += $(MODULENAME).o


# The minus -C /lib/modules/$(shell uname -r)/build specifies a
# directory that is created when a kernel is installed on a
# machine. The $(shell uname -r) component ensures that we choose the
# directory corresponding to the currently running kernel. The
# directory contains the build environment for modules that will be
# used with the currently running kernel.
#
# The M=$(PWD) argument supplies the directory in which this makefile
# appears and so the invocation of make uses that environment to build
# the module represented by this makefile against the appropriate
# kernel.
#
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
