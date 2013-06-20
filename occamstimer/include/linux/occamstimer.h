#ifndef OCCAMSTIMER_H
#define OCCAMSTIMER_H



/* 
 * ===============================================
 *             OCCAMSTIMER Data Structures
 * ===============================================
 */

/* Convienient constant name */
#define OT_MODULE_NAME "occamstimer"

/* The maximum buffer size of each work packet  */
#define OT_MAX_WORK_SIZE 1024 /* 1 kb */



/**
 * @OT_SETUP: The initial state of the workqueue during which the user
 *            program can populate the workqueue to create desired
 *            initial conditions. While in this state no work items
 *            are serviced and no timers are active.
 *
 * @OT_RUNNING: The timer is active and when it expires the item at front of
 *              the pending queue will be serviced.
 *
 * @OT_PAUSED: The current timer has been intialized and started, but
 *             user operation has stopped the countdown of the pending
 *             timer.
 *
 * @OT_ITEM_SERVICE: This status occurs after timer fire when in the
 *                   callback routine for the timer and the item at
 *                   the front of the pending queue will be serviced.
 *
 * @OT_FINISHED: During the last OT_ITEM_SERVICE state the pending
 *               queue was determined to be empty so we are therefore
 *               finished. The timer is not restarted. 
 *
 */
enum occamstimer_status{
	OT_SETUP = 0,
	OT_RUNNING,
	OT_STOPPED,
	OT_ITEM_SERVICE,
	OT_FINISHED,       
};


enum occamstimer_action {
	OT_ACTION_START = 0,
	OT_ACTION_PAUSE,
};


/* 
 * This header file is used by both kernel code and user code. The
 * portion of the header used by kernel code is concealed from user
 * code by the __KERNEL__ ifdef.
 */
#ifdef __KERNEL__


/* 
 * We want to put datastream instrumentation points in the code in
 * general, but we also want to illustrate how to create a
 * "production" version of the module without instrumentation so we
 * control inclusion of the DSKI header with this kernel config
 * symbol.
 * 
 * Note that we also define that we also define the OCCAMSTIMER_DEBUG
 * statement is used in place of printk to include narrative debug
 * statements in the datastream output.
 */
#ifdef CONFIG_KUSP_OCCAMSTIMER_DSKI
#include <linux/kusp/dski.h>
#define OT_DEBUG(fmt, args...) DSTRM_DEBUG(OCCAMSTIMER, DEBUG, fmt, ## args)
#define OT_EVENT(ename) DSTRM_EVENT(OCCAMSTIMER, ename, 0)
#define OT_INFO(info) DSTRM_DEBUG(OCCAMSTIMER, DEBUG, "[%d] %s\n", __line__, info)
#else
#define OT_DEBUG(fmt, args...) \
	printk(strcat(strcat("[OCCAMSTIMER:%d] ", fmt), "\n"), __LINE__, ##args)

#define OT_EVENT(ename) OT_DEBUG(#ename)
#define OT_INFO(info) OT_DEBUG(info)
#endif /* CONFIG_KUSP_OCCAMSTIMER_DSKI */





#endif /* __KERNEL__*/

/* 
 * ===============================================
 *             Public API Functions
 * ===============================================
 */

/*
 * In this example we have a few different module attributes to which
 * we would like to have get/set access. Instead of doubling the
 * amount of possible IOCTL calls, we will instead have a get/set flag
 * within each generic attribute call. The internal routines will then
 * need to read this flag to know whether the user would like to get
 * or set the pertinent attribute.
 */
enum occamstimer_attr_cmd {
	OT_ATTR_GET = 0,
	OT_ATTR_SET,
	OT_ATTR_ADD,
};



/*
 * There typically needs to be a struct definition for each flavor of
 * IOCTL call.  
 */

/*
 * A helper struct that will store the two parameters for the
 * "add_work" ioctl call. We do this so that we can keep a general
 * format for each type of IOCTL call by continuing to use "cmd" and
 * "value" attribtues at the top level.
 */
struct occamstimer_ioctl_work_params {
	char                          data[OT_MAX_WORK_SIZE];
	struct timespec               exec_int;
};

typedef struct occamstimer_ioctl_work_s {
	enum occamstimer_attr_cmd             cmd;
	struct occamstimer_ioctl_work_params  value; 
} occamstimer_ioctl_work_t;


typedef struct occamstimer_ioctl_status_s {
	enum occamstimer_attr_cmd     cmd;
	enum occamstimer_status       value;
} occamstimer_ioctl_status_t;


typedef struct occamstimer_ioctl_action_s {
	enum occamstimer_attr_cmd    cmd;
	enum occamstimer_action      value;

} occamstimer_ioctl_action_t;

/* 
 * This generic union allows us to make a more generic IOCTRL call
 * interface. Each per-IOCTL-flavor struct should be a member of this
 * union.
 */
typedef union occamstimer_ioctl_param_u {
	occamstimer_ioctl_work_t          work;
	occamstimer_ioctl_status_t        status;
	occamstimer_ioctl_action_t        action;
} occamstimer_ioctl_param_union;


/* 
 * Used by _IOW to create the unique IOCTL call numbers. It appears
 * that this is supposed to be a single character from the examples I
 * have looked at so far.
 */
#define OCCAMSTIMER_MAGIC 'o'

/*
 * For each flavor of IOCTL call you will need to make a macro that
 * calls the _IOW() macro. This macro is just a macro that creates a
 * unique ID for each type of IOCTL call. It uses a combination of bit
 * shifting and OR-ing of each of these arguments to create the
 * (hopefully) unique constants used for IOCTL command values.
 */
#define OCCAMSTIMER_IOCTL_WORK  \
	_IOW(OCCAMSTIMER_MAGIC, 1, occamstimer_ioctl_work_t)
#define OCCAMSTIMER_IOCTL_STATUS \
	_IOW(OCCAMSTIMER_MAGIC, 2, occamstimer_ioctl_status_t)
#define OCCAMSTIMER_IOCTL_ACTION \
	_IOW(OCCAMSTIMER_MAGIC, 3, occamstimer_ioctl_action_t)

#endif /* OCCAMSTIMER_H */
