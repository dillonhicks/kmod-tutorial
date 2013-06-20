#ifndef USERPROG_H
#define USERPROG_H

#include <linux/tabularasa.h>

struct user_params {
        char    name[TABULARASA_MAX_NAME_LEN];
	int     pprint;	

};

#endif	/*USERPROG_H */
