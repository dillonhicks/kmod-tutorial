/*
 * Tabula Rasa User Program 
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
#include <linux/tabularasa.h>
#include <tabularasa.h>

#include "truser.h"

/* User cmd line parameters */
struct user_params Params = {
        .name = "potatochip pancakes!",
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
			{"name",           required_argument, NULL, 'n'},
			{"pprint",           no_argument,       NULL, 'p'},
			{"help",             no_argument,       NULL, 'h'},
			{NULL, 0, NULL, 0}
		};

		/**
		 * c contains the last in the lists above corresponding to
		 * the long argument the user used.
		 */
		c = getopt_long(argc, argv, "n:p", long_options, &option_index);

		if (c == -1)
			break;
		switch (c) {
		case 0:
			switch (option_index) {
			case 0:
//				printf(help_string, argv[0]);
				exit(EXIT_SUCCESS);
				break;
			}
			break;
			
		case 'n':
		        snprintf(Params.name, TABULARASA_MAX_NAME_LEN, optarg);
		        break;

		case 'h':
			error = 1;
			break;

		}
	}
	
	if (error) {

		exit(EXIT_FAILURE);
	}
}


int main(int argc, char** argv){
	
        int fd;

	process_options(argc, argv);		

	fd = tabularasa_open();


	if (fd < 0) {
		printf("There was an error opening Tabularasa.\n");
		exit(EXIT_FAILURE);
	}

	if (tabularasa_set_name(fd, Params.name)) {
		printf("There was an error setting the name of tabularasa.\n");
		exit(EXIT_FAILURE);
	}

	if (tabularasa_close(fd)) {
		printf("There was an error closing Tabularasa.\n");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
