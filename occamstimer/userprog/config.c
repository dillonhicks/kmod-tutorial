#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "xhashconf.h"


static workspec_t* get_new_workspec();

//Public:
workspec_t* get_config(kusp_config *config, int *t_count)
{
	
	kusp_config *workspec_config = NULL;
	kusp_config *kc_elem = NULL;
	kusp_attr   *kc_attr, *kc_tmp_attr;
	workspec_t  *wspec_list = NULL;
	workspec_t  *wspec_elem = NULL;

	*t_count = 0;
	
	if (!config) {
		printf("error: null config\n");

		return NULL;

	} else {

		HASH_FIND_STR(config->children, CONFIG_WORKSPEC_NAME, workspec_config); 
		
	}

	if (!workspec_config) {
		printf("error: failed to find an workitems in config\n");
		return NULL;		
	}


	DL_FOREACH(workspec_config, kc_elem){

		wspec_elem = get_new_workspec();				

		HASH_ITER(hh, kc_elem->attributes, kc_attr, kc_tmp_attr) {

			if (!strcmp(kc_attr->name, "data"))
				 strcpy(wspec_elem->data, kc_attr->value);
			else if (!strcmp(kc_attr->name, "tv_sec"))
				wspec_elem->exec_int.tv_sec = atol(kc_attr->value);
			else if (!strcmp(kc_attr->name, "tv_nsec"))
				wspec_elem->exec_int.tv_nsec = atol(kc_attr->value);
			else
				printf("warning: unknown workspec_t attribute %s", 
				       kc_attr->name);
		}

		(*t_count)++;
		DL_APPEND(wspec_list, wspec_elem);

	}

	return wspec_list;
}

void free_config(workspec_t *head)
{
	workspec_t *cur = NULL, *next = NULL;

	DL_FOREACH_SAFE(head, cur, next){
		DL_DELETE(head, cur);
	}
}


/**
 *
 */
void pprint_workspec(workspec_t *head){
 	
 	workspec_t *ws;
	int count = 0;
	
	DL_FOREACH(head, ws) {

		printf("[%d] workspec_t\n", count);
		printf("-----------------\n");
		printf("data:\t\t%s\n", ws->data);
		printf("exec interval:\t%lds %ldns\n", 
		       (unsigned long)ws->exec_int.tv_sec, ws->exec_int.tv_nsec);  

		printf("has next:\t%s\n", ws->next != NULL ? "yes" : "no");		
		printf("\n");       	       
		
		count++;
	}
	
	printf("workspec count: %d\n\n", count); 	
}


static workspec_t* get_new_workspec()
{
	workspec_t *ws = (workspec_t*)malloc(sizeof(workspec_t));     

	strcpy(ws->data, "");
	ws->exec_int.tv_sec = 0;
	ws->exec_int.tv_nsec = 0;


	ws->next = NULL;
	ws->prev = NULL;
	return ws;
}


