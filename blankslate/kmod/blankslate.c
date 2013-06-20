/*
 * blankslate.c - Example kmod utilizing ioctl and sysfs
 *
 *
 *  This module borrows heavily from the kobject example code at:
 *    <kr>/samples/kobject/kobject-example.c
 */
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kobject.h>
#include <linux/string.h>

/* 
 * This is a relative include which assumes that it is being compiled
 * from <kr>/examples/kmods/blankslate/kmod. Thus, the relative
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
#include "../include/linux/blankslate.h"



/* 
 * Protects concurrent reading/writing attributes in the blankslate
 * module.
 */
struct mutex blankslate_mutex;

/* 
 * This data is accessed from the user level and thus might be
 * accessed concurrently. This is the string that is emitted as the
 * value in /sys/devices/blankslate/name.
 */
char blankslate_module_name[BLANKSLATE_MAX_NAME_LEN];


/*
 * This data is accessed from the user level and thus might be
 * accessed concurrently.  This value is emitted in
 * /sys/devices/blankslate/rutabaga_count.
 */
int blankslate_rutabaga_count;


/* 
 * ===============================================
 *                Public Interface
 * ===============================================
 */

/*
 * Sets blankslate_module_name to the name parameter. 
 */
int blankslate_set_name(const char *name) {

	int ret = 0;
	
	printk("Changing name %s->%s", blankslate_module_name, name); 

	/* 
	 * Grab the modules mutex in order to protect against
	 * concurrent sets 
	 */
	mutex_lock(&blankslate_mutex);

	/* 
	 * Set the new name of the module utilizing only up to
	 * BLANKSLATE_MAX_NAME_LEN characters from the new_name
	 * string. If there was an error with the snprintf() call then
	 * it returns a negative value. Otherwise, it returns the
	 * number of bytes copied to the destination char array.
	 */
	if (snprintf(blankslate_module_name, BLANKSLATE_MAX_NAME_LEN, name) < 0) {
	      ret = -EINVAL;
	}

	mutex_unlock(&blankslate_mutex);	

	return ret;

}

/*
 * Sets blankslate_rutabaga_count 
 */
int blankslate_set_rutabaga_count(int count) {

  
  printk("Changing rutabaga count %d->%d", blankslate_rutabaga_count, count); 

  /* 
   * Grab the module's mutex in order to protect against
   * concurrent sets 
   */
  mutex_lock(&blankslate_mutex);
  
  /* 
   * Nothing fancy here since it is just integer assignment.
   */
  
  blankslate_rutabaga_count = count;
  
  mutex_unlock(&blankslate_mutex);	
  
  return 0;
}


/* 
 * ===============================================
 *                IOCTL Interface
 * ===============================================
 */

/*
 * Generic open call that will always be successful since there is no
 * extra setup we need to do for this module.
 */
static int
blankslate_open(struct inode *inode, struct file *file) {
	return 0;
}


static int
blankslate_close(struct inode *inode, struct file *file) {
	return 0;
}

static long
blankslate_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int                               ret = 0;
	blankslate_ioctl_param_union      local_param;

	if (copy_from_user
	    ((void *)&local_param, (void *)ioctl_param, _IOC_SIZE(ioctl_num)))
		return  -ENOMEM;

	switch (ioctl_num) {

	case BLANKSLATE_IOCTL_SET_NAME:
	{
		char  *name = local_param.set_name.name;
		printk("blankslate: attempting to set name of module.\n");

		ret = blankslate_set_name(name);
		break;
	}
	
	case BLANKSLATE_IOCTL_SET_RUTABAGA_COUNT:
	{
		int count = local_param.set_rutabaga_count.count;

		ret =  blankslate_set_rutabaga_count(count);
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
 *            Sysfs Interface
 * ===============================================
 */

/*
 * The easiest approach is for each variable you wish to expose to
 * sysfs should have a show() and store() routine. These are the
 * kobject/sysfs versions of the typical get/set routines.
 *
 */

static ssize_t blankslate_module_name_show(struct kobject *kobj, 
					   struct kobj_attribute *attr, char *buf)
{
	int ret = 0;
	
	printk("reading blankslate_module_name");
	
	mutex_lock(&blankslate_mutex);	
	ret =  sprintf(buf, "%s\n", blankslate_module_name);
	mutex_unlock(&blankslate_mutex);

	return ret;
}

static ssize_t blankslate_module_name_store(struct kobject *kobj, 
					    struct kobj_attribute *attr,
					    const char *buf, size_t count)
{
	/* 
	 * We alread have a function to properly set the name of the
	 * module so let's use that instead of duplicating code.
	 */
	blankslate_set_name(buf);

	/* 
	 * I am unsure why the <variable>_store routines seem to
	 * require to return the count argument. Possibly something to
	 * do with KSets? I'm not sure at the moment.
	 */
	return count;
}

static struct kobj_attribute blankslate_module_name_attr =
	__ATTR(blankslate_module_name,       /* Var to which to add attrs  */
	       0666,			     /* mode (a la chmod) */
	       blankslate_module_name_show,  /* sysfs callback to read var  */
	       blankslate_module_name_store);/* sysfs callback to write var */



/*
 *
 */
static ssize_t blankslate_rutabaga_count_show(struct kobject *kobj, 
					      struct kobj_attribute *attr, char *buf)
{
	int ret = 0;
	
	printk("reading blankslate_rutabaga_count");
	
	mutex_lock(&blankslate_mutex);	
	ret =  sprintf(buf, "%d\n", blankslate_rutabaga_count);
	mutex_unlock(&blankslate_mutex);

	return ret;
}

static ssize_t blankslate_rutabaga_count_store(struct kobject *kobj, 
					       struct kobj_attribute *attr,
					       const char *buf, size_t count)
{

	int rutabaga_count = 0;

	/* 
	 * One downside to sysfs is all data is passed around as
	 * buffers that are character arrays. To get the desired
	 * integer value from the buffer we must use the sscanf
	 * function to convert the character data in the buffer to an
	 * integer.
	 */
	sscanf(buf, "%d", &rutabaga_count);

	blankslate_set_rutabaga_count(rutabaga_count);

	/* 
	 * I am unsure why the <variable>_store routines seem to
	 * require to return the count argument. Possibly something to
	 * do with KSets? I'm not sure at the moment.
	 */
	return count;
}


static struct kobj_attribute blankslate_rutabaga_count_attr =
	__ATTR(blankslate_rutabaga_count,       /* Var to which to add attrs  */
	       0666,			        /* mode (a la chmod) */
	       blankslate_rutabaga_count_show,  /* sysfs callback to read var  */
	       blankslate_rutabaga_count_store);/* sysfs callback to write var */


/*
 * Create a group of attributes so that we can create and destory them all
 * at once.
 */
static struct attribute *blankslate_kobj_attrs[] = {
	&blankslate_module_name_attr.attr,
	&blankslate_rutabaga_count_attr.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group blankslate_kobj_attr_group = {
	.attrs = blankslate_kobj_attrs,
};

/*
 * The root kobj for this module. 
 */
static struct kobject *blankslate_kobj;



/* 
 * ===============================================
 *            Module init/exit 
 * ===============================================
 */

/* 
 * The file_operations struct is an instance of the standard character
 * device table entry. We choose to initialize only the open, release,
 * and unlock_ioctl elements since these are the only functions we use
 * in this module.
 */
struct file_operations
blankslate_dev_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = blankslate_ioctl,
	.open           = blankslate_open,
	.release        = blankslate_close,
};

/* 
 * This is an instance of the miscdevice structure which is used in
 * the blankslate_init routine as a part of registering the module
 * when it is loaded. 
 * 
 * The device type is "misc" which means that it will be assigned a
 * static major number of 10. We deduced this by doing ls -la /dev and
 * noticed several different entries we knew to be modules with major
 * number 10 but with different minor numbers.
 * 
 */
static struct miscdevice
blankslate_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = BLANKSLATE_MODULE_NAME,
	.fops  = &blankslate_dev_fops,
};


/*
 * This routine is executed when the module is loaded into the
 * kernel. I.E. during the insmod command.
 */
static int
__init blankslate_init(void)
{
	int ret = 0;

	/*
	 * Attempt to register the module as a misc. device with the
	 * kernel.
	 */
	ret = misc_register(&blankslate_misc);
	
	if (ret < 0) {
		/* Registration failed so give up. */
		goto out;
	}


	/*
	 * Create a simple kobject with the name of "blankslate",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	blankslate_kobj = kobject_create_and_add(BLANKSLATE_MODULE_NAME, kernel_kobj);
	if (!blankslate_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	ret = sysfs_create_group(blankslate_kobj, &blankslate_kobj_attr_group);
	if (ret)
		kobject_put(blankslate_kobj);


	/* Initializing the shared data */
	strcpy(blankslate_module_name, BLANKSLATE_MODULE_NAME);

	blankslate_rutabaga_count = 0;

	mutex_init(&blankslate_mutex);

	printk("blankslate module installed\n");

out:
	return ret;
}

/*
 * This code is executed when the module is being removed from the
 * kernel during the rmmod command.
 */
static void
__exit blankslate_exit(void)
{ 
	mutex_destroy(&blankslate_mutex);

	misc_deregister(&blankslate_misc);

	printk("blankslate module uninstalled\n");
}

module_init(blankslate_init);
module_exit(blankslate_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple module example with SYSFS and IOCTL");
MODULE_AUTHOR("Dillon Hicks <hhicks@ittc.ku.edu>");
