#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <blankslate.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/unistd.h>

#include <linux/blankslate.h>


int blankslate_open(void) {

	int fd = open("/dev/blankslate", 'w');
	
	if (fd < 0)
		return -fd;
	
	return fd;
}



int blankslate_close(int fd) {
	
	int ret = 0;
	ret = close(fd);
	return ret;
}


int blankslate_set_name(int blankslate_fd, char *name){

	int ret = 0;
	
	blankslate_ioctl_set_name_t ioctl_args;
		
	if (strlen(name) > BLANKSLATE_MAX_NAME_LEN) {
	  return -EINVAL;
	}

	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	strcpy(ioctl_args.name, name);

	ret = ioctl(blankslate_fd, BLANKSLATE_IOCTL_SET_NAME, &ioctl_args);

	return ret;
}


int blankslate_reset_name(int blankslate_fd) {
        int ret = 0;
  
        blankslate_ioctl_set_name_t ioctl_args;
  
	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	strcpy(ioctl_args.name, BLANKSLATE_MODULE_NAME);

	ret = ioctl(blankslate_fd, BLANKSLATE_IOCTL_SET_NAME, &ioctl_args);

        return ret;
}


int blankslate_set_rutabaga_count(int blankslate_fd, int count){

	int ret = 0;
	
	blankslate_ioctl_set_rutabaga_count_t ioctl_args;

	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	ioctl_args.count = count;

	ret = ioctl(blankslate_fd, BLANKSLATE_IOCTL_SET_RUTABAGA_COUNT, &ioctl_args);

	return ret;
}
