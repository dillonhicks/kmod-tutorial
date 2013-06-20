/*
 * helloworld.c - Example kmod utilizing ioctl and procfs
 *
 */
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/types.h>

/* 
 * This is a relative include which assumes that it is being compiled
 * from $KUSPROOT/examples/kmods/helloworld/kmod. Thus, the relative
 * pathname referes the corresponding directory in the example source
 * tree.
 * 
 * The Makefile in the kernel code that is called by the Makefile for
 * this module does not have an obvious way to set the
 * C_INCLUDE_PATH. The kernel module Makefile seems to hardcode the
 * include directory to be relative to the kernel area. Therefore, to
 * be able to include the header file we created for this module I
 * have been forced to use a relative pathname.
 *
 * If this module were installed to the system (via an RPM for
 * example) this assertion would be false because the header file
 * would be placed in the kernel include area.
 */
#include "../include/linux/helloworld.h"


/*
 * The number of times to append the line "Hello World!" to the
 * /proc/helloworld buffer.
 */
atomic_t helloworld_message_count = ATOMIC_INIT(0);

/*
 * Required Proc File-system Struct
 *
 * Used to map entry into proc file table upon module insertion
 */
struct proc_dir_entry *helloworld_proc_entry;


/* 
 * ===============================================
 *                Public Interface
 * ===============================================
 */

/*
 * Increament the message count. This could just as easily be done
 * within the helloworld_ioctl() function.
 */
static void 
helloworld_inc_message_count(void){
	atomic_inc(&helloworld_message_count);
}


/* 
 * ===============================================
 *                IOCTL Interface
 * ===============================================
 */

/*
 * Generic open call that will always be successful since there is no
 * extra setup we need to do for this module as far
 */
static int
helloworld_open(struct inode *inode, struct file *file) {
	return 0;
}


static int
helloworld_close(struct inode *inode, struct file *file) {
	return 0;
}

static long
helloworld_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int                               ret = 0;
	helloworld_ioctl_param_union      local_param;

	printk("helloworld_ioctl()\n");

	if (copy_from_user
	    ((void *)&local_param, (void *)ioctl_param, _IOC_SIZE(ioctl_num)))
		return  -ENOMEM;

	switch (ioctl_num) {

	case HELLOWORLD_IOCTL_INCREMENT:
	{
		helloworld_inc_message_count();
		ret = 0;
		break;
	}

	default:
	{
		printk("ioctl: no such command\n");
		ret = -EINVAL;
	}
	} /* end of switch(ioctl_num) */

	
	return ret;
}




/* 
 * ===============================================
 *            Proc File Table Interface
 * ===============================================
 */

/* 
 * The type and order of the parameters is standard for a /proc/ read
 * routine, at least, and possibly for all read routines but we didn't
 * go and check this assuption yet.
 *
 */
static int helloworld_proc_read(char *buffer,
				char **buffer_location,
				off_t offset, int buffer_length, int *eof, void *data) {
	int ret;
	int i;

	/* 
	 * Obtain a local copy of the atomic variable that is
	 * gaurenteed not to change while in the buffer formatting
	 * logic..
	 */
	int message_count = atomic_read(&helloworld_message_count);

	printk("reading helloworld proc entry.\n");
	
	/* 
	 * We give all of our information in one go, so if the user
	 * asks us if we have more information the answer should
	 * always be no.
	 *
	 * This is important because the standard read function from
	 * the library would continue to issue the read system call
	 * until the kernel replies that it has no more information,
	 * or until its buffer is filled.
	 * 
	 */
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size This
		 * assumes that the buffer passed in is big enough to
		 * hold what we put in it. More defensive code would
		 * check the input buffer_length parameter to check
		 * the validity of that assumption.
		 *
		 * Note that the return value from this call (number
		 * of characters written to the buffer) from this will
		 * be added to the current offset at the file
		 * descriptor level managed by the system code that is
		 * called by this routine.
		 */
		
		/* 
		 * Make sure we are starting off with a clean buffer;
		 */
		strcpy(buffer, "");
		for (i = 0; i < message_count; i ++) {
			/* 
			 * Now that we are sure what the buffer
			 * contains we can just append the message the
			 * desired number of times. The third argument
			 * to strncat() makes sure we do not go over
			 * the length of the buffer.
			 */
			buffer = strncat(buffer, "Hello World!\n", 
				(buffer_length - strlen(buffer)) );		
		}
		ret = strlen(buffer);
	}

	return ret;
}


/* 
 * The file_operations struct is an instance of the standard character
 * device table entry. We choose to initialize only the open, release,
 * and unlock_ioctl elements since these are the only functions we use
 * in this module.
 */
struct file_operations
helloworld_dev_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = helloworld_ioctl,
	.open           = helloworld_open,
	.release        = helloworld_close,
};

/* 
 * This is an instance of the miscdevice structure which is used in
 * the helloworld_init routine as a part of registering the module
 * when it is loaded.
 * 
 * The device type is "misc" which means that it will be assigned a
 * static major number of 10. We deduced this by doing ls -la /dev and
 * noticed several different entries we knew to be modules with major
 * number 10 but with different minor numbers.
 * 
 */
static struct miscdevice
helloworld_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = HELLOWORLD_MODULE_NAME,
	.fops  = &helloworld_dev_fops,
};


/*
 * This routine is executed when the module is loaded into the
 * kernel. I.E. during the insmod command.
 *
 */
static int
__init helloworld_init(void)
{
	int ret = 0;

	/*
	 * Attempt to register the module as a misc. device with the
	 * kernel.
	 */
	ret = misc_register(&helloworld_misc);
	
	if (ret < 0) {
		/* Registration failed so give up. */
		goto out;
	}

	/* 
	 * Creating an entry in /proc with the module name as the file
	 * name (/proc/helloworld). The S_IRUGO | S_IWUGO flags set
	 * the permissions to 666 (rw,rw,rw).
	 */
	helloworld_proc_entry = create_proc_entry(HELLOWORLD_MODULE_NAME, 
						  S_IRUGO | S_IWUGO, NULL);

	if (helloworld_proc_entry == NULL) {
		/*
		 * When the create_proc_entry fails we return a No
		 * Memory Error. Whether ENOMEM is accurate or a
		 * approximate is separate issue but we chose to use
		 * an existing error number.
		 */
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * We are hooking up the helloworld_proc_read routine because
	 * we intend to support only read operations on the procfile.
	 */
	helloworld_proc_entry->read_proc = helloworld_proc_read;

	printk("helloworld module installed\n");
	


out:
	return ret;
}

static void
__exit helloworld_exit(void)
{ 
        remove_proc_entry(HELLOWORLD_MODULE_NAME, helloworld_proc_entry);

	misc_deregister(&helloworld_misc);

	printk("helloworld module uninstalled\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple module example with procfs and IOCTL");
MODULE_AUTHOR("Dillon Hicks");
