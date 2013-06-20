/*
 * Blank Slate User Program 
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
#include <linux/blankslate.h>
#include <blankslate.h>

#include "bsuser.h"

/* User cmd line parameters */
struct user_params Params = {
        .name = "",
        .count = 0
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
			{"count",         required_argument, NULL, 'c'},
			{"help",             no_argument,     NULL, 'h'},
			{NULL, 0, NULL, 0}
		};

		/**
		 * c contains the last in the lists above corresponding to
		 * the long argument the user used.
		 */
		c = getopt_long(argc, argv, "n:c", long_options, &option_index);

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
		        snprintf(Params.name, BLANKSLATE_MAX_NAME_LEN, optarg);
		        break;

		case 'c':
			sscanf(optarg, "%d", &Params.count);
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

	fd = blankslate_open();


	if (fd < 0) {
		printf("There was an error opening Blankslate.\n");
		exit(EXIT_FAILURE);
	}

	if (strlen(Params.name))
		if (blankslate_set_name(fd, Params.name)) {
			printf("There was an error setting the name of blankslate.\n");
			exit(EXIT_FAILURE);
		}
	
	if (Params.count >= 0)
		if (blankslate_set_rutabaga_count(fd, Params.count)) {
			printf("There was an error setting the rutabaga "
			       "count of blankslate.\n");
			exit(EXIT_FAILURE);
		}


	if (blankslate_close(fd)) {
		printf("There was an error closing Blankslate.\n");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
