SHELL:=/bin/bash

NAMES=	skynet\
	john-conner\
	t-800\
	terminator


prog:
	-rm -rf build
	-mkdir build
	@cd build ;\
	cmake .. ;\
	make ;


run:
	@cat /proc/tabularasa
	@(for NAME in $(NAMES); do \
		./build/userprog/truser --name=$$NAME ;\
		cat /proc/tabularasa ;\
	done;)


kmod:
	make -C./kmod clean module reinstall