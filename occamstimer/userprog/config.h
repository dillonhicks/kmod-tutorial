#ifndef OTCONF_H
#define OTCONF_H

#include <linux/occamstimer.h>
#include "xhashconf.h"

#define CONFIG_WORKSPEC_NAME "workitem"

typedef struct workspec_s {
	
	char              data[OT_MAX_WORK_SIZE];
	struct timespec   exec_int;
	
	struct workspec_s *next;	
	struct workspec_s *prev;

} workspec_t;

workspec_t *get_config(kusp_config *config, int *t_count);
void free_config(workspec_t *head);
void pprint_workspec(workspec_t *head);

#endif	/* OTCONF_H */
