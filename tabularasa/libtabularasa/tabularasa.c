#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <tabularasa.h>
#include <errno.h>
#include <linux/tabularasa.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/unistd.h>



int tabularasa_open(void) {

	int fd = open("/dev/tabularasa", 'w');
	
	if (fd < 0)
		return -fd;
	
	return fd;
}



int tabularasa_close(int fd) {
	
	int ret = 0;
	ret = close(fd);
	return ret;
}


int tabularasa_set_name(int tabularasa_fd, char *name){

	int ret = 0;
	
	tabularasa_ioctl_set_t ioctl_args;
		
	if (strlen(name) > TABULARASA_MAX_NAME_LEN) {
	  return -EINVAL;
	}

	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	strcpy(ioctl_args.name, name);

	ret = ioctl(tabularasa_fd, TABULARASA_IOCTL_SET, &ioctl_args);

	return ret;
}



int tabularasa_reset_name(int tabularasa_fd) {
        int ret = 0;
  
        tabularasa_ioctl_set_t ioctl_args;
  
	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	strcpy(ioctl_args.name, TABULARASA_MODULE_NAME);

	ret = ioctl(tabularasa_fd, TABULARASA_IOCTL_SET, &ioctl_args);

        return ret;
}
