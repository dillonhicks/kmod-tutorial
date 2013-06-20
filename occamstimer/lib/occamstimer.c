#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <occamstimer.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/unistd.h>




/**
 * Retrieve the file descriptor for the occamstimer device. Needed to
 * use IOCTL.
 */
int occamstimer_open(void) {
	return open("/dev/occamstimer", 'w');
}


/**
 * Close
 */
int occamstimer_close(int fd) {
	return close(fd);
}


/**
 * Add a workitem to the occamstimer pending work queue.
 * 
 * @fd: The file descriptor to /dev/occamstimer
 * @data: The character buffer representing the work to do
 *
 * @exec_int: The simulated execution interval that the simulated
 *            device would take to process @data.
 */
int occamstimer_add_work(int fd, char *data, struct timespec *exec_int) {

	int ret = 0;
	
	occamstimer_ioctl_work_t ioctl_args;
		
	if (strlen(data) > OT_MAX_WORK_SIZE) {
	  return -EINVAL;
	}

	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	ioctl_args.cmd = OT_ATTR_ADD;
	
	strcpy(ioctl_args.value.data, data);
	
	ioctl_args.value.exec_int.tv_sec = exec_int->tv_sec;
	ioctl_args.value.exec_int.tv_nsec = exec_int->tv_nsec;
	
	ret = ioctl(fd, OCCAMSTIMER_IOCTL_WORK, &ioctl_args);

	return ret;
}


/**
 * Get completed work from occamstimer
 * 
 * @fd: The file descriptor to /dev/occamstimer
 * @data: The character buffer representing the completed work
 */
int occamstimer_get_work(int fd, char *data) {

	int ret = 0;
	
	occamstimer_ioctl_work_t ioctl_args;
		
	if (strlen(data) < OT_MAX_WORK_SIZE) {
	  return -EINVAL;
	}

	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	ioctl_args.cmd = OT_ATTR_GET;
	
	ret = ioctl(fd, OCCAMSTIMER_IOCTL_WORK, &ioctl_args);

	if (!ret)
		strcpy(data, ioctl_args.value.data);

	return ret;
}


/**
 * Sets the state of the device. Note that this function should not be
 * used and is included for educational purposes. Use the
 * occamstimer_<action>_device functions instead.
 * 
 * @fd: The file descriptor to /dev/occamstimer
 * @status: The status
 */
int occamstimer_set_status(int fd, enum occamstimer_status status) {

	int ret = 0;
	
	occamstimer_ioctl_status_t ioctl_args;
		
	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	ioctl_args.cmd = OT_ATTR_SET;
	
	ioctl_args.value = status;
	
	ret = ioctl(fd, OCCAMSTIMER_IOCTL_STATUS, &ioctl_args);

	return ret;
}



int occamstimer_get_status(int fd, enum occamstimer_status *status) {

	int ret = 0;
	
	occamstimer_ioctl_status_t ioctl_args;

	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	ioctl_args.cmd = OT_ATTR_GET;
	
	ret = ioctl(fd, OCCAMSTIMER_IOCTL_STATUS, &ioctl_args);
	
	if (!ret)
		*status = ioctl_args.value;

	return ret;
}


int __occamstimer_do_action(int fd, enum occamstimer_action action) {

	int ret = 0;
	
	occamstimer_ioctl_action_t ioctl_args;
		
	memset(&ioctl_args, 0, sizeof(ioctl_args));
	
	ioctl_args.cmd = OT_ATTR_SET;
	
	ioctl_args.value = action;
	
	ret = ioctl(fd, OCCAMSTIMER_IOCTL_ACTION, &ioctl_args);

	return ret;

}

int occamstimer_start_device(int fd) {
	return __occamstimer_do_action(fd, OT_ACTION_START);
}


int occamstimer_pause_device(int fd) {
	return __occamstimer_do_action(fd, OT_ACTION_PAUSE);
}







