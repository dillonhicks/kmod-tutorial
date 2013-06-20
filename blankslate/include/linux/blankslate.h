#ifndef BLANKSLATE_H
#define BLANKSLATE_H

/* 
 * ===============================================
 *             BLANKSLATE Data Structures
 * ===============================================
 */

/* Maximum set name length */
#define BLANKSLATE_MAX_NAME_LEN 100

/* Convienient constant name */
#define BLANKSLATE_MODULE_NAME "blankslate"

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
 * Note that we also define that we also define the BLANKSLATE_DEBUG
 * statement is used in place of printk to include narrative debug
 * statements in the datastream output.
 */
#ifdef CONFIG_KUSP_BLANKSLATE_DSKI
#include <linux/kusp/dski.h>
#define BLANKSLATE_DEBUG(fmt, args...) DSTRM_DEBUG(BLANKSLATE, DEBUG, fmt, ## args)
#else
#define BLANKSLATE_DEBUG(fmt, args...) 
#endif /* CONFIG_KUSP_BLANKSLATE_DSKI */


/* 
 * Protects reading/writing attributes in the blankslate
 * module. Granted this is only blankslate_module_name at the moment.
 */
extern struct mutex blankslate_mutex;

/* 
 * This data is accessed from the user level and thus might be
 * accessed concurrently. 
 */
extern char blankslate_module_name[BLANKSLATE_MAX_NAME_LEN];

extern int blankslate_rutabaga_count;

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
typedef struct blankslate_ioctl_set_name_s {
	char name[BLANKSLATE_MAX_NAME_LEN];
} blankslate_ioctl_set_name_t;

typedef struct blankslate_ioctl_set_rutabaga_count_s {
	int count;
} blankslate_ioctl_set_rutabaga_count_t;


/* 
 * This generic union allows us to make a more generic IOCTRL call
 * interface. Each per-IOCTL-flavor struct should be a member of this
 * union.
 */
typedef union blankslate_ioctl_param_u {
	blankslate_ioctl_set_name_t              set_name;
	blankslate_ioctl_set_rutabaga_count_t    set_rutabaga_count;
} blankslate_ioctl_param_union;


/* 
 * Used by _IOW to create the unique IOCTL call numbers. It appears
 * that this is supposed to be a single character from the examples I
 * have looked at so far.
 */
#define BLANKSLATE_MAGIC 't'

/*
 * For each flavor of IOCTL call you will need to make a macro that
 * calls the _IOW() macro. This macro is just a macro that creates a
 * unique ID for each type of IOCTL call. It uses a combination of bit
 * shifting and OR-ing of each of these arguments to create the
 * (hopefully) unique constants used for IOCTL command values.
 */
#define BLANKSLATE_IOCTL_SET_NAME  \
	_IOW(BLANKSLATE_MAGIC, 1, blankslate_ioctl_set_name_t)
#define BLANKSLATE_IOCTL_SET_RUTABAGA_COUNT \
	_IOW(BLANKSLATE_MAGIC, 2, blankslate_ioctl_set_rutabaga_count_t)

#endif /* BLANKSLATE_H */
