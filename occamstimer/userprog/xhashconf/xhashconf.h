#ifndef XHASHCONF_H
#define XHASHCONF_H

#include "uthash/utlist.h"
#include "uthash/uthash.h"

#define NODE_NAME_LENGTH 20
#define NODE_ATTR_LENGTH 20
#define NODE_ATTR_VALUE_LENGTH (NODE_ATTR_LENGTH * 5)
#define PPRINT_SPACES_PER_LEVEL 4

typedef struct _kusp_attr
{
	char name[NODE_ATTR_LENGTH];
	char value[NODE_ATTR_VALUE_LENGTH]; 

	UT_hash_handle hh;
} kusp_attr;	


typedef struct _kusp_config
{ 
	char name[NODE_NAME_LENGTH];

	kusp_attr *attributes; /* hash table with attributes */
	char *content; /* any text content from the xml node */


	struct _kusp_config *children;

	/* list of the nodes at this point */
	struct _kusp_config *prev;
	struct _kusp_config *next;
	UT_hash_handle hh; /* enables uthash to hash this data structure */

} kusp_config;


kusp_config *kusp_parse_xml_config(char *filename);
kusp_attr *kusp_get_attr(kusp_config *node, char *attr_name);
kusp_config *kusp_get_node(kusp_config *node, char *node_name);
void kusp_dump_config(kusp_config *config, char *filename);
void kusp_free_config(kusp_config *config);
void kusp_pprint_config(kusp_config *config);

#endif	/* XHASHCONF_H */
