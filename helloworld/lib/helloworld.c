#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/unistd.h>


#include <linux/helloworld.h>
#include <helloworld.h>


int helloworld_open(void) {

	int fd = open("/dev/helloworld", 'w');
	
	if (fd < 0)
		return -fd;
	
	return fd;
}


int helloworld_close(int fd) {
	
	int ret = 0;
	ret = close(fd);
	return ret;
}


int helloworld_inc(int hw_fd){

	int ret = 0;	

	helloworld_ioctl_inc_t ioctl_args;
		
	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	ioctl_args.placeholder = LIBHELLOWORLD_MAGIC;

	ret = ioctl(hw_fd, HELLOWORLD_IOCTL_INCREMENT, &ioctl_args);

	return ret;
}


