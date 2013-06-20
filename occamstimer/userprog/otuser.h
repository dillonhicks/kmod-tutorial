#ifndef OTUSER_H
#define OTUSER_H

#include <linux/occamstimer.h>

#include "config.h"
#include "xhashconf.h"

#define help_string "\
	\n\nusage %s --config=<filename>  [--pprint] [--help]\n\n\
\t--config=\t\tthe configuration file of work items\n\
\t--pprint\t\tpretty print the confiugration after parsing\n\
\t--help\t\t\tthis menu\n\n"


struct user_params {
	kusp_config  *config;
	workspec_t   *workspec_config;
        int           workspec_count;	
	int           pprint;	
};


#endif	/* OTUSER_H */
