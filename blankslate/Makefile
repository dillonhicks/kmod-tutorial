SHELL:=/bin/bash

NAME_DEFAULT = "blankslate"
RUTABAGA_DEFAULT = 0

NAMES=	skynet \
	john-conner \
	t-800 \
	terminator \
	napoleon

RUTABAGA_COUNTS=1234 \
		411 \
		121 \
		756 \
		0 \
		23 

SYSFS_BS_PATH = /sys/kernel/blankslate
SYSFS_NAME_PATH = $(SYSFS_BS_PATH)/blankslate_module_name
SYSFS_RUTABAGA_PATH = $(SYSFS_BS_PATH)/blankslate_rutabaga_count

.PHONY: prog reset run-prog run-script run kmod clean

prog:
	-rm -rf build
	-mkdir build
	@cd build ;\
	cmake .. ;\
	make ;


reset:
	@echo "========================================"
	@echo "Resetting module attributes to initial values"
	@echo "========================================"

	@echo -n $(NAME_DEFAULT) > $(SYSFS_NAME_PATH)
	@echo -n $(RUTABAGA_DEFAULT) > $(SYSFS_RUTABAGA_PATH)
	@echo -n "blankslate_module_name = " 
	@cat $(SYSFS_NAME_PATH)
	@echo -n "blankslate_rutabaga_count = "
	@cat $(SYSFS_RUTABAGA_PATH)
	@echo 
	@echo

run-prog:
	@echo "========================================"
	@echo "Modifying blankslate module variables using the user program w/ IOCTL."   
	@echo "========================================"
	@echo 
	@echo "setting blankslate_module_name"
	@echo "****************************************"
	@(for NAME in $(NAMES); do \
		./build/userprog/bsuser --name=$$NAME ;\
		cat $(SYSFS_NAME_PATH) ;\
	done;)
	@echo 
	@echo "setting blankslate_rutabaga_count"
	@echo "****************************************"
	@(for COUNT in $(RUTABAGA_COUNTS); do \
		./build/userprog/bsuser --count=$$COUNT ;\
		cat $(SYSFS_RUTABAGA_PATH) ;\
	done;)
	@echo

run-script:
	@echo "========================================"
	@echo "Modifying blankslate module using a bash commands."
	@echo "========================================"
	@echo 
	@echo "setting blankslate_module_name"
	@echo "****************************************"
	@(for NAME in $(NAMES); do \
		echo -n $$NAME > $(SYSFS_NAME_PATH) ;\
		cat $(SYSFS_NAME_PATH) ;\
	done;)
	@echo 
	@echo "setting blankslate_rutabaga_count"
	@echo "****************************************"
	@(for COUNT in $(RUTABAGA_COUNTS); do \
		echo -n $$COUNT > $(SYSFS_RUTABAGA_PATH) ;\
		cat $(SYSFS_RUTABAGA_PATH) ;\
	done;)
	@echo
	@echo

run: reset run-prog reset run-script 


kmod:
	make -C./kmod clean module reinstall

clean:
	rm -rfv build/