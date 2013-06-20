SHELL:=/bin/bash

INCREMENTS = 1 2 3 4 5 6


prog:
	-rm -rf build
	-mkdir build
	@cd build ;\
	cmake .. ;\
	make ;


run:
	@cat /proc/helloworld
	@echo 

	@(for INCREMENT in $(INCREMENTS); do \
		./build/userprog/hwuser --inc=$$INCREMENT ;\
		cat /proc/helloworld ;\
		echo ;\
	done;)


clean:
	-rm -rfv build/ 
	-rm -fv $(shell find . | grep ~$$)
	-cd kmod/; make clean

