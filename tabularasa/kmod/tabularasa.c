/*
 * tabularasa.c - Example kmod utilizing ioctl and procfs
 *
 */
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/err.h>

/* 
 * This is a relative include which assumes that it is being compiled
 * from $KUSPROOT/examples/kmods/tabularasa/kmod. Thus, the relative
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
#include "../include/linux/tabularasa.h"


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple module example with procfs and IOCTL");
MODULE_AUTHOR("Dillon Hicks");


/* 
 * Protects concurrent reading/writing attributes in the tabularasa
 * module. Granted this is only tabularasa_module_name at the moment.
 */
struct mutex tabularasa_mutex;

/* 
 * This data is accessed from the user level and thus might be
 * accessed concurrently. This is the string that is emitted as part
 * of the string in /proc/tabularasa.
 */
char tabularasa_module_name[TABULARASA_MAX_NAME_LEN];

/*
 * Required Proc File-system Struct
 *
 * Used to map entry into proc file table upon module insertion
 */
struct proc_dir_entry *tabularasa_proc_entry;




/* 
 * ===============================================
 *                Public Interface
 * ===============================================
 */

/*
 * Sets tabularasa_module_name to the name parameter. 
 */
int tabularasa_set_name(char *name) {

	int ret = 0;
	
	printk("Changing name %s->%s", tabularasa_module_name, name); 

	/* 
	 * Grab the modules mutex in order to protect against
	 * concurrent sets 
	 */
	mutex_lock(&tabularasa_mutex);

	/* 
	 * Set the new name of the module utilizing only up to
	 * TABULARASA_MAX_NAME_LEN characters from the new_name
	 * string. If there was an error with the snprintf() call then
	 * it returns a negative value. Otherwise, it returns the
	 * number of bytes copied to the destination char array.
	 */
	if (snprintf(tabularasa_module_name, TABULARASA_MAX_NAME_LEN, name) < 0) {
	      ret = -EINVAL;
	}

	mutex_unlock(&tabularasa_mutex);	

	return ret;

}

/* To be implemented, maybe put something stupid in here, drop a Q
 * somewhere in the string
 */
int tabularasa_transform_name(int flags) {
  return 0;
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
tabularasa_open(struct inode *inode, struct file *file) {
	return 0;
}


static int
tabularasa_close(struct inode *inode, struct file *file) {
	return 0;
}

static long
tabularasa_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int                               ret = 0;
	tabularasa_ioctl_param_union      local_param;

	if (copy_from_user
	    ((void *)&local_param, (void *)ioctl_param, _IOC_SIZE(ioctl_num)))
		return  -ENOMEM;

	switch (ioctl_num) {

	case TABULARASA_IOCTL_SET:
	{
		char  *name = local_param.set.name;
		printk("tabularasa: attempting to set name of module.\n");

		ret = tabularasa_set_name(name);
		break;
	}
	
	case TABULARASA_IOCTL_TRANSFORM:
	{
		int flags = local_param.transform.flags;

		ret =  tabularasa_transform_name(flags);
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
static int tabularasa_proc_read(char *buffer,
				char **buffer_location,
				off_t offset, int buffer_length, int *eof, void *data) {
	int ret;
	
	printk("Reading tabularasa proc entry.\n");
	
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
		mutex_lock(&tabularasa_mutex);
		ret = sprintf(buffer, "Hello World! I am %s\n", tabularasa_module_name);
		mutex_unlock(&tabularasa_mutex);
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
tabularasa_dev_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = tabularasa_ioctl,
	.open           = tabularasa_open,
	.release        = tabularasa_close,
};

/* 
 * This is an instance of the miscdevice structure which is used in
 * the tabularasa_init routine as a part of registering the module
 * when it is loaded. 
 * 
 * The device type is "misc" which means that it will be assigned a
 * static major number of 10. We deduced this by doing ls -la /dev and
 * noticed several different entries we knew to be modules with major
 * number 10 but with different minor numbers.
 * 
 */
static struct miscdevice
tabularasa_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = TABULARASA_MODULE_NAME,
	.fops  = &tabularasa_dev_fops,
};


/*
 * This routine is executed when the module is loaded into the
 * kernel. I.E. during the insmod command.
 */
static int
__init tabularasa_init(void)
{
	int ret = 0;

	/*
	 * Attempt to register the module as a misc. device with the
	 * kernel.
	 */
	ret = misc_register(&tabularasa_misc);
	
	if (ret < 0) {
		/* Registration failed so give up. */
		goto out;
	}

	/* Creating an entry in /proc with the module name as the file
	 * name (/proc/tabularasa).
	 */
	tabularasa_proc_entry = create_proc_entry(TABULARASA_MODULE_NAME, S_IRUGO | S_IWUGO, NULL);

	if (tabularasa_proc_entry == NULL) {
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
	 * We are hooking up the tabularasa_proc_read routine because
	 * we intend to support only read operations on the procfile.
	 */
	tabularasa_proc_entry->read_proc = tabularasa_proc_read;

	/* Initializing the shared data to the module name */
	strcpy(tabularasa_module_name, TABULARASA_MODULE_NAME);

	mutex_init(&tabularasa_mutex);

	printk("Tabula Rasa module installed\n");

out:
	return ret;
}

static void
__exit tabularasa_exit(void)
{ 
        remove_proc_entry(TABULARASA_MODULE_NAME, tabularasa_proc_entry);

	mutex_destroy(&tabularasa_mutex);

	misc_deregister(&tabularasa_misc);
	
	printk("Tabula Rasa module uninstalled\n");
}

module_init(tabularasa_init);
module_exit(tabularasa_exit);

