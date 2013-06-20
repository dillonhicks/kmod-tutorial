/*
 * occamstimer.c - Example kmod utilizing HRTimers, and IOCTL
 */
#include <asm/siginfo.h>	/* siginfo */
#include <asm/uaccess.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/kusp/dski.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/module.h> 
#include <linux/string.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/spinlock_types.h>

/* 
 * This is a relative include which assumes that it is being compiled
 * from $KUSPROOT/examples/kmods/occamstimer/kmod. Thus, the relative
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
#include "../include/linux/occamstimer.h"


/*
 * This/These structure(s) creates the list that we will use as a
 * workqueue. Note that we do not use the linux kernel standard
 * workqueues since this is an example of timer use and the workqueue
 * structure at linux/workqueue.h has many assumptions that, while
 * generally good, hide many features at a higher level of abstraction
 * than would be desired for an example of how to use kernel timers in
 * order to simulate a device driver to do some work.
 */ 


/**
 * @data: The buffer containing the specific "work"
 *
 * @exec_int: Execution interval of the workitem. This is the
 *           simulation duration that the workitem would take.
 */
struct occamstimer_workitem {
	char                data[OT_MAX_WORK_SIZE];
	struct timespec     exec_int;
	struct list_head    ent;
};


/**
 * @lock: atomic spin_lock that protects the physically concurrent
 *        access to this structure. Interrupt concurrency in
 *        controlled on the thread side by enabling and disabling
 *        interrupts.
 * 
 * @timer: Periodic timer who's handler routine operates on pending
 *         workitems. This handler routine simulates the ISR for a
 *         generic device driver which is operating on the
 *         workqueue. The expiration of a pending timer is the time at
 *         which the item at the front of the workqueue should be
 *         serviced.
 *
 * @status: State variable indicating the current state of the
 *          workqueue. The value of this variable is drawn from the
 *          enum occamstimer_status.
 *
 * @pending: The pending work items are enqueued to this list
 *           structure. During the execution of the timer handler
 *           routine, the first item will be dequeued and
 *           serviced.
 *
 * @done: Work items serviced from the pending queue are enqueued on
 *        to this list after being serviced.
 */
struct occamstimer_workqueue {
	spinlock_t                lock;
	struct hrtimer            timer;
	enum occamstimer_status   status; 
	struct list_head          pending;
	struct list_head          done;	
};

/**
 * The one instance of struct occamstimer_workqueue that will serve as
 * the workqueue. An possible exercise would be to generalize this
 * module to contain many workqueues (e.g. using a list). This
 * modified module would, in turn, simulate many devices of the same
 * type controlled by the same basice device driver.
 */
struct occamstimer_workqueue ot_workqueue;


/* 
 * ===============================================
 *                Public Interface
 * ===============================================
 */

/**
 * Assumption: Calling context holds the queue lock
 */
static void 
__occamstimer_get_status(enum occamstimer_status *status) {
	
        OT_EVENT(FUNC_GET_STATUS_2);
	*status = ot_workqueue.status;
}

/** 
 * 
 */
static int
occamstimer_get_status(enum occamstimer_status *status) {

	OT_EVENT(FUNC_GET_STATUS_1);

	spin_lock(&ot_workqueue.lock);

	__occamstimer_get_status(status);

	spin_lock(&ot_workqueue.lock);
	

	return 0;
}

/** 
 * Assumption: Calling context holds the queue lock
 */
static void
__occamstimer_set_status(enum occamstimer_status new_status) {
	OT_EVENT(FUNC_SET_STATUS_2);
	OT_DEBUG("status==%d\n", ot_workqueue.status);
	OT_DEBUG("new_status==%d\n", new_status);
	ot_workqueue.status = new_status;

}

/** 
 * 
 */
static int
occamstimer_set_status(enum occamstimer_status new_status) {

	OT_EVENT(FUNC_SET_STATUS_1);
	/*
	 * Since we want to control concurrent access to our shared
	 * workqueue we disable interrupts/preemption before
	 * setting/changing the status.
	 */
	spin_lock_irq(&ot_workqueue.lock);

	__occamstimer_set_status(new_status);
	/* Done reading the status so restore the irq flags to their
	 * previous state. */
	spin_lock_irq(&ot_workqueue.lock);


	return 0;
	
}



/**
 * 
 */
static int
occamstimer_start(void) {

	int ret = 0;

	struct occamstimer_workitem *work_ptr;
	struct timespec             now_time;

	OT_EVENT(FUNC_START);



	spin_lock(&ot_workqueue.lock);
	
	switch (ot_workqueue.status) {

	case OT_SETUP:
	case OT_FINISHED:
		OT_INFO("case=setup||finished");
		if (list_empty(&ot_workqueue.pending)) {
			OT_INFO("list_empty(pending)!");
			spin_unlock(&ot_workqueue.lock);
			break;
		}
		
		/* Get the item at the front of the queue */
		work_ptr = list_first_entry(&ot_workqueue.pending, 
					    struct occamstimer_workitem,
					    ent);

		/* Get the current ktime */
		ktime_get_ts(&now_time);

		/* Calculate the execution interval on the absolute
		 * timeline for the workitem as now + exec_int using a
		 * function that incorporates the rollover of
		 * nanoseconds to seconds.
		 */
		set_normalized_timespec(&work_ptr->exec_int,
					now_time.tv_sec + work_ptr->exec_int.tv_sec,
					now_time.tv_nsec + work_ptr->exec_int.tv_nsec);


		
		/* OT_DEBUG("now=%d:%d\n", now_time.tv_sec, now_time.tv_nsec); */
		/* OT_DEBUG("exec_int=%l:%l\n", work_prt->exec_int.tv_sec, work_ptr->exec_int.tv_nsec); */
		
		__occamstimer_set_status(OT_RUNNING);

		/* Start the timer */
		hrtimer_start(&ot_workqueue.timer, 
			      timespec_to_ktime(work_ptr->exec_int), HRTIMER_MODE_ABS);
		
		spin_unlock(&ot_workqueue.lock);		
		break;

	case OT_STOPPED:
		OT_INFO("case=stop");
		/* If somehow the queue became empty while stopped,
		 * flip out since this indicates undesired operation
		 * and a serious logic since the queue items should
		 * not be able to be removed whiled stopped..
		 */
		BUG_ON(list_empty(&ot_workqueue.pending));

		/* Get the item at the front of the queue */
		work_ptr = list_first_entry(&ot_workqueue.pending, 
					    struct occamstimer_workitem,
					    ent);


		/* Get the current ktime */
		ktime_get_ts(&now_time);

		/* Calculate the execution interval on the absolute
		 * timeline for the workitem as now + exec_int using a
		 * function that incorporates the rollover of
		 * nanoseconds to seconds.
		 */
		set_normalized_timespec(&work_ptr->exec_int,
					now_time.tv_sec + work_ptr->exec_int.tv_sec, 
					now_time.tv_nsec + work_ptr->exec_int.tv_nsec);


		__occamstimer_set_status(OT_RUNNING);

		/* Start the timer. The exec_int should be set for the
		 * remaining time of the timer stopped during the last
		 * "pause".
		 */
		hrtimer_start(&ot_workqueue.timer, 
			      timespec_to_ktime(work_ptr->exec_int), HRTIMER_MODE_ABS);


		spin_unlock(&ot_workqueue.lock);
		break;

	default:
		OT_INFO("case==default");
		spin_unlock(&ot_workqueue.lock);
		ret = -EINVAL;
		break;
	} 

	
	return ret;

}



/**
 * if OT_RUNNING, stop the timer and change the status to OT_STOPPED.
 */
static int
occamstimer_pause(void) {
	int ret = 0;
	
	OT_EVENT(FUNC_PAUSE);

	spin_lock_irq(&ot_workqueue.lock);

	switch (ot_workqueue.status) {
	case OT_RUNNING:
		/* Note: a smarter routine would get the remaining
		 * time for the timer using hrtimer_get_remaining()
		 * and set the item at the front of the pending
		 * workqueue to have the remaining time as its new
		 * execution interval so on the next
		 * "occamstimer_start()" the timer would be started
		 * with the remaining time instead of the full
		 * execution interval again.
		 */
		hrtimer_cancel(&ot_workqueue.timer);
		__occamstimer_set_status(OT_STOPPED);
		spin_unlock_irq(&ot_workqueue.lock);
		break;
	case OT_STOPPED:
	case OT_FINISHED:
		/* When stopped or finished no change of state. Unlock
		 * the lock and break..
		 */
		spin_unlock_irq(&ot_workqueue.lock);
		break;

	case OT_ITEM_SERVICE:
		spin_unlock_irq(&ot_workqueue.lock);
		WARN(1, "Tried to pause while in timer callback. This should not happen\n.");
		/* "Operation not permitted"s (EPERM) or "Device busy" (EBUSY)?  */
		ret = -EPERM;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	
	
	return ret;
}

/**
 * Add the workitem to the pending queue.
 * 
 * Assumption: calling context holds the queue lock.
 *
 * @work_ptr: The pointer to the workitem to add to the pending queue 
 */
static void
__occamstimer_add_work(struct occamstimer_workitem *work_ptr) {	

	OT_EVENT(FUNC_ADD_WORK_2);
      	/* 
	 * Add the new item to the end of the list in order to provide
	 * queueing semantics.
	 */
	list_add_tail(&work_ptr->ent, &ot_workqueue.pending);

}


/**
 * Allocates kernel memory for the workitem and initializes its data,
 * exec_int, and ent fields.
 * 
 * Assumption: calling context holds the queue lock.
 *
 * @work_ptr: The pointer to the workitem to initialize 
 * @data: The data buffer that represents the work to do.
 * @exec_int: The simulated execution interval to complete the work.
 */
static int
__occamstimer_workitem_init(struct occamstimer_workitem *work_ptr, 
			    char *data, struct timespec *exec_int) {
	int ret = 0;
	
	OT_EVENT(FUNC_WORKITEM_INIT);

	//printk("strlen(data)==%d", strlen(data));
	OT_DEBUG("strlen(data)==%d", strlen(data));
	if (strlen(data) > OT_MAX_WORK_SIZE) {
		/* The workitem is too large, so return an overflow
		 * error. */
		ret = -EOVERFLOW;
		goto err;
	}
			

	if (work_ptr == NULL) {
		/* Assume that if we cannot allocate memory then there
		 * is none. */
		OT_INFO("workitem memory kmalloc failed");
		ret = -ENOMEM;
		goto err;
	}	

	/* Copy the data to our workitem */
	strcpy(work_ptr->data, data);
	
	work_ptr->exec_int.tv_sec = exec_int->tv_sec;
	work_ptr->exec_int.tv_nsec = exec_int->tv_nsec;

	/* /\*  */
	/*  * Initialize the listhead corresponding to the entry for */
	/*  * new_work that will be on the workqueue's listhead. */
	/*  *\/ */
	/* INIT_LIST_HEAD(&work_ptr->ent); */

err:
	return ret;

}

/**
 * Add the workitem specified by the arguments to the pending queue,
 * if able.
 */
static int
occamstimer_add_work(char *data, struct timespec *exec_int) {

	int     ret = 0;
 	struct occamstimer_workitem *work_ptr;


	OT_EVENT(FUNC_ADD_WORK_1);	

	/* 
	 * Allocate kernel memory where we will store the new
	 * workitem.
	 */
	work_ptr = (struct occamstimer_workitem *) \
	  kmalloc(sizeof(struct occamstimer_workitem), GFP_KERNEL);
	
	/* Try to initialize the workitem  */
	ret = __occamstimer_workitem_init(work_ptr, data, exec_int);
	
	if (ret) {
		/* 'data' was too large or memory allocation failed.  */	
		OT_INFO("workitem init err");
		goto err;
	}

	spin_lock(&ot_workqueue.lock);
      	
	switch (ot_workqueue.status) {
	case OT_SETUP:
	case OT_STOPPED:
	case OT_FINISHED:
		__occamstimer_add_work(work_ptr);
		
	default:
		ret = -EINVAL;
	}
		    
	spin_unlock(&ot_workqueue.lock);

err:

	return ret;
}


/**
 * Service the specific workitem
 */
static void
occamstimer_do_work(struct occamstimer_workitem *work_ptr){

	OT_EVENT(FUNC_DO_WORK);
	/* TODO: add extra stuff? A dummy loop? */
	OT_DEBUG("[%d] data: %s\n", __LINE__, work_ptr->data);
	list_move_tail(&work_ptr->ent, &ot_workqueue.done);

}


/**
 * 
 */
static int
occamstimer_get_work(char *data) {

	int ret = 0;
	struct occamstimer_workitem *work_ptr;

	OT_EVENT(FUNC_GET_WORK);

	spin_lock(&ot_workqueue.lock);
	if (list_empty(&ot_workqueue.done)) {
		data = NULL;
		spin_unlock(&ot_workqueue.lock);
		goto out;
	}

	/* Get the list entry for the first workitem in the done
	 * queue. */
	work_ptr = list_first_entry(&ot_workqueue.done, 
				    struct occamstimer_workitem, ent);

	/* Delete the entry from the done list */
	list_del(&work_ptr->ent);

	/* Finished modifying the queue so give up the lock. */
	spin_unlock(&ot_workqueue.lock);

	strcpy(data, work_ptr->data);

	/* Finally remember to free the workitem pointed to by
	 * work_ptr since we kmalloc'd it earlier. */
	kfree(work_ptr);

out:

	return ret;
}





/* 
 * ===============================================
 *                 Interrupt Handlers
 * ===============================================
 */

/**
 * The timer's handler function/
 */
static enum hrtimer_restart
occamstimer_workqueue_timer_callback(struct hrtimer *timer) {

	struct occamstimer_workitem    *work_ptr;

	OT_EVENT(FUNC_WORKQUEUE_TIMER_CALLBACK);

	spin_lock_irq(&ot_workqueue.lock);
	
	if (unlikely(ot_workqueue.status != OT_RUNNING)) {
	  
		OT_DEBUG("status==%d\n", ot_workqueue.status);
       
		WARN(1, "Timer callback activated when (status != OT_RUNNING). "
		        "This should not happen - timer will not be restarted.\n"); 
		goto norestart;

	} 
	
	__occamstimer_set_status(OT_ITEM_SERVICE);

	if (unlikely(list_empty(&ot_workqueue.pending))) {
		WARN(1, "Timer callback activated when (pending queue is empty). "
		        "This should not happen - timer will not be restarted.\n"); 
		__occamstimer_set_status(OT_FINISHED);
		goto norestart;
	}

	
	occamstimer_do_work(list_first_entry(&ot_workqueue.pending, 
					     struct occamstimer_workitem, ent));
	
	if (unlikely(list_empty(&ot_workqueue.pending))) {
		__occamstimer_set_status(OT_FINISHED);
		goto norestart;
	}
	
	

	/* Get the item at the front of the queue */
	work_ptr = list_first_entry(&ot_workqueue.pending, 
				    struct occamstimer_workitem,
				    ent);
	
	/* Set the new expiration of the timer to the current time
	 * (ktime_get()) plus the execution interval fo the next work
	 * item */
	hrtimer_forward(timer, ktime_get(), timespec_to_ktime(work_ptr->exec_int));

	__occamstimer_set_status(OT_RUNNING);

	spin_unlock_irq(&ot_workqueue.lock);

	return HRTIMER_RESTART;





norestart:
	spin_unlock_irq(&ot_workqueue.lock);
	return HRTIMER_NORESTART;
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
occamstimer_open(struct inode *inode, struct file *file) {
	OT_EVENT(FUNC_OPEN);
	return 0;
}


static int
occamstimer_close(struct inode *inode, struct file *file) {
	OT_EVENT(FUNC_CLOSE);
	return 0;
}

static long
occamstimer_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int                               ret = 0;
	occamstimer_ioctl_param_union      local_param;

	OT_EVENT(FUNC_IOCTL);

	if (copy_from_user
	    ((void *)&local_param, (void *)ioctl_param, _IOC_SIZE(ioctl_num)))
		return  -ENOMEM;

	switch (ioctl_num) {

	case OCCAMSTIMER_IOCTL_WORK:
	{
		if (local_param.work.cmd == OT_ATTR_GET) {
			ret = occamstimer_get_work(local_param.work.value.data);
			/* TODO: Copy back to user */
		} else if (local_param.work.cmd == OT_ATTR_ADD) {
			ret = occamstimer_add_work(local_param.work.value.data, 
						   &local_param.work.value.exec_int);
		} else{			
			ret = -EINVAL;
		}
			
		break;
	}

	case OCCAMSTIMER_IOCTL_STATUS:
	{
		if (local_param.status.cmd == OT_ATTR_GET) {
			ret = occamstimer_get_status(&local_param.status.value);
			/* TODO: Copy back to user */
		} else if (local_param.status.cmd == OT_ATTR_SET) {
			ret = occamstimer_set_status(local_param.status.value);
		} else {
			ret = -EINVAL;
		}
			
		break;

	}

	case OCCAMSTIMER_IOCTL_ACTION:
		
		if (local_param.action.value == OT_ACTION_START)
			ret = occamstimer_start();
		else if (local_param.action.value == OT_ACTION_PAUSE)
			ret = occamstimer_pause();
		else 
			WARN(1, "Undefined action for occamstimer.\n");
						
		break;
		

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
occamstimer_dev_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = occamstimer_ioctl,
	.open           = occamstimer_open,
	.release        = occamstimer_close,
};


/* 
 * This is an instance of the miscdevice structure which is used in
 * the occamstimer_init routine as a part of registering the module
 * when it is loaded. 
 * 
 * The device type is "misc" which means that it will be assigned a
 * static major number of 10. We deduced this by doing ls -la /dev and
 * noticed several different entries we knew to be modules with major
 * number 10 but with different minor numbers.
 * 
 */
static struct miscdevice
occamstimer_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = OT_MODULE_NAME,
	.fops  = &occamstimer_dev_fops,
};


/**
 * This routine is executed when the module is loaded into the
 * kernel. I.E. during the insmod command.
 */
static int
__init occamstimer_init(void)
{
	int ret = 0;

	/*
	 * Attempt to register the module as a misc. device with the
	 * kernel.
	 */
	ret = misc_register(&occamstimer_misc);
		
	if (ret < 0) {
		/* Registration failed so give up. */
		goto out;
	}


	/* Initializing ot_workqueue */

	ot_workqueue.status = OT_SETUP;

	spin_lock_init(&ot_workqueue.lock);

	/* 
	 * The lists that will function the pending work queue and
	 * completed work queue.
	 */
	INIT_LIST_HEAD(&ot_workqueue.pending);
	INIT_LIST_HEAD(&ot_workqueue.done);
	
	/* 
	 * Timer - Note that we initialize the timer to absolute
	 * timeframe mode and set the appropriate handler routine, but
	 * do not start it.
	 */
	hrtimer_init(&ot_workqueue.timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);

	ot_workqueue.timer.function = occamstimer_workqueue_timer_callback;

	printk("occamstimer module installed\n");

out:
	return ret;
}

/*
 * This code is executed when the module is being removed from the
 * kernel during the rmmod command.
 */
static void
__exit occamstimer_exit(void)
{ 
	misc_deregister(&occamstimer_misc);

	printk("occamstimer module uninstalled\n");
}

module_init(occamstimer_init);
module_exit(occamstimer_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple module example with HRTimers and IOCTL");
MODULE_AUTHOR("Dillon Hicks <hhicks@ittc.ku.edu>");
