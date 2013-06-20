SHELL:=/bin/bash

NAME_DEFAULT = "occamstimer"

SYSFS_OT_PATH = /sys/kernel/occamstimer

OTUSER = ./build/userprog/otuser

CONFIG = ./userprog/small_queue.xml

.PHONY: prog reset run kmod clean

prog:
	-rm -rf build
	-mkdir build
	@cd build ;\
	cmake .. ;\
	make ;



run:
	@echo "========================================"
	$(OTUSER) --config=$(CONFIG)
	@echo "========================================"
	exit 1


exec:
	pushd kmod/ ; \
	make clean module ;\
	-sudo make uninstall ;\
	sudo make install ;\
	popd ;\
	pushd build/userprog ;\
	./otuser -c small_queue.xml ;\
	popd;


kmod:
	make -C./kmod clean module reinstall


clean:
	rm -rfv build/