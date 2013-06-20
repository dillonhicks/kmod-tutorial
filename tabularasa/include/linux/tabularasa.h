#ifndef TABULARASA_H
#define TABULARASA_H

/* 
 * ===============================================
 *             TABULARASA Data Structures
 * ===============================================
 */

/* Maximum set name length */
#define TABULARASA_MAX_NAME_LEN 100

/* Convienient constant name */
#define TABULARASA_MODULE_NAME "tabularasa"

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
 * Note that we also define that we also define the TABULARASA_DEBUG
 * statement is used in place of printk to include narrative debug
 * statements in the datastream output.
 */
#ifdef CONFIG_KUSP_TABULARASA_DSKI
#include <linux/kusp/dski.h>
#define TABULARASA_DEBUG(fmt, args...) DSTRM_DEBUG(TABULARASA, DEBUG, fmt, ## args)
#else
#define TABULARASA_DEBUG(fmt, args...) 
#endif /* CONFIG_KUSP_TABULARASA_DSKI */



/* 
 * Protects reading/writing attributes in the tabularasa
 * module. Granted this is only tabularasa_module_name at the moment.
 */
extern struct mutex tabularasa_mutex;

/* 
 * This data is accessed from the user level and thus might be
 * accessed concurrently. 
 */
extern char tabularasa_module_name[TABULARASA_MAX_NAME_LEN];


/*
 * Required Proc File-system Struct
 *
 * Used to map entry into proc file table upon module insertion
 */
extern struct proc_dir_entry *tabularasa_proc_entry;

#endif /* __KERNEL__*/

/* 
 * ===============================================
 *             Public API Functions
 * ===============================================
 */


/*
 * There typically needs to be a struct definition for each flavor of
 * IOCTL call. 
 */
typedef struct tabularasa_ioctl_set_s {
	char name[TABULARASA_MAX_NAME_LEN];
} tabularasa_ioctl_set_t;

typedef struct tabularasa_ioctl_transform_s {
	int flags;
} tabularasa_ioctl_transform_t;


/* 
 * This generic union allows us to make a more generic IOCTRL call
 * interface. Each per-IOCTL-flavor struct should be a member of this
 * union. 
 */
typedef union tabularasa_ioctl_param_u {
	tabularasa_ioctl_set_t      set;
	tabularasa_ioctl_transform_t       transform;
} tabularasa_ioctl_param_union;


/* 
 * Used by _IOW to create the unique IOCTL call numbers. It appears
 * that this is supposed to be a single character from the examples I
 * have looked at so far. 
 */
#define TABULARASA_MAGIC 't'

/*
 * For each flavor of IOCTL call you will need to make a macro that
 * calls the _IOW() macro. This macro is just a macro that creates a
 * unique ID for each type of IOCTL call. It uses a combination of bit
 * shifting and OR-ing of each of these arguments to create the
 * (hopefully) unique constants used for IOCTL command values.
 */
#define TABULARASA_IOCTL_SET	   _IOW(TABULARASA_MAGIC, 1, tabularasa_ioctl_set_t)
#define TABULARASA_IOCTL_TRANSFORM _IOW(TABULARASA_MAGIC, 2, tabularasa_ioctl_transform_t)

#endif /* TABULARASA_H */
