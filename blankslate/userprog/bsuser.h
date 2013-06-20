#ifndef BSUSER_H
#define BSUSER_H

#include <linux/blankslate.h>

struct user_params {
        char    name[BLANKSLATE_MAX_NAME_LEN];
	int     count;	

};

#endif	/* BSUSER_H */
