/*
 * Occam's Timer User Program 
 *
 * Author: Dillon Hicks (hhicks@ittc.ku.edu)
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define __USE_GNU
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <time.h>
#include <string.h>
#include <linux/occamstimer.h>
#include <occamstimer.h>

#include "otuser.h"


/* User cmd line parameters */
struct user_params Params = {
	.workspec_config = NULL,
	.workspec_count = 0,
	.pprint = 0
};


/**
 *  This subroutine processes the command line options 
 */
void process_options (int argc, char *argv[])
{

	int error = 0;
	int c;
	for (;;) {
		int option_index = 0;

		static struct option long_options[] = {
			{"config",           required_argument, NULL, 'c'},
			{"pprint",           no_argument,       NULL, 'p'},
			{"help",             no_argument,       NULL, 'h'},
			{NULL, 0, NULL, 0}
		};

		/**
		 * c contains the last in the lists above corresponding to
		 * the long argument the user used.
		 */
		c = getopt_long(argc, argv, "c:p", long_options, &option_index);

		if (c == -1)
			break;
		switch (c) {
		case 0:
			switch (option_index) {
			case 0:
				printf(help_string, argv[0]);
				exit(EXIT_SUCCESS);
				break;
			}
			break;
			
		case 'c':
			/* Parse the whole config using the standard config parser */
			Params.config = kusp_parse_xml_config(optarg);

			/* Get the thread section for ease of use */
			Params.workspec_config = \
				get_config(Params.config, &Params.workspec_count);

			if (!Params.workspec_config || !Params.workspec_count){
				
				printf("parse config: failed to parse any"
                                        "workspec from config file %s (config is"
                                        "null? %s) \n", 
				       optarg, 
				       Params.workspec_config == NULL ? "yes" : "no");
			}
			
			break;
		case 'p':
			Params.pprint = 1;
			break;

		case 'h':
			error = 1;
			break;

		}
	}
	
	if (error) {
		printf(help_string, argv[0]);
		exit(EXIT_FAILURE);
	}
}




int main(int argc, char** argv){
	
        int fd;
	workspec_t *workitem;
	
	process_options(argc, argv);		
	
	if (Params.pprint)
		pprint_workspec(Params.workspec_config);


	fd = occamstimer_open();
	

	if (fd < 0) {
		printf("There was an error opening /dev/occamstimer.\n");
		exit(EXIT_FAILURE);
	}
	
	workitem = Params.workspec_config; 
	while(workitem) {

		if(!occamstimer_add_work(fd, workitem->data, &workitem->exec_int))
			printf("error adding work\n");
		else
			printf("added work\n");

		workitem = workitem->next;
	}

	
	/* start the timer */
	occamstimer_start_device(fd);
	

	if (occamstimer_close(fd)) {

		printf("There was an error closing Occamstimer.\n");

		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
