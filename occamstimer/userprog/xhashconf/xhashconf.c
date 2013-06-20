#ifndef XHASHCONF_C
#define XHASHCONF_C

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "uthash/utlist.h"
#include "xhashconf.h"

static kusp_config* traverse_config(xmlNode *node);
static kusp_config* alloc_config(xmlNode *node);
static kusp_config* add_config(kusp_config *root, kusp_config *child);
static xmlNodePtr build_xml_config(kusp_config *config, xmlNodePtr node);



static void __kusp_pprint_config_R(kusp_config *elem, int level) {
	
	kusp_config *child_list, *tmp_child_list, *child_elem ;
	kusp_attr *attr, *tmp_attr;
	
	printf("[%d] %s\n", level, elem->name);

	
	HASH_ITER(hh, elem->attributes, attr, tmp_attr) {
		printf("       %s = %s\n", attr->name, attr->value);
	}	       

	HASH_ITER(hh, elem->children, child_list, tmp_child_list) {
		
		DL_FOREACH(child_list, child_elem) {		
			__kusp_pprint_config_R(child_elem, level + 1);
		}	
	}
	
}

void kusp_pprint_config(kusp_config *config){
	__kusp_pprint_config_R(config, 0);
}

kusp_config* kusp_parse_xml_config(char *filename){
	kusp_config *config = NULL;
	/* stores the parsed document tree */
	xmlDoc *doc = NULL;

	/* the root node is the head of a document */
	xmlNode *root = NULL;

 	/* initialize the library and check for ABI mismatches
	 * between how it was compiled and the actual shared lib
	 * imported
	 */
	LIBXML_TEST_VERSION

	doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL){
#ifdef XHASHCONF_DEBUG
		printf("error: failed to parse configuration file %s\n", filename);
#endif	/* XHASHCONF_DEBUG */

 	} else {
		/* we need the root element from the document tree */
		root = xmlDocGetRootElement(doc);

		/* if we traverse the root node, we won't get any of
		 * our <thread/> items, so we traverse the children
		 * instead 
		 */
		config = traverse_config(root);
 	}

	/* free resources libxml2 has allocated throughout the course here */
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return config;
} 


kusp_config* kusp_get_node(kusp_config *node, char *node_name)
{ 
	kusp_config *temp;

	HASH_FIND_STR(node, node_name, temp);

	return temp;
}


kusp_attr* kusp_get_attr(kusp_config *node, char *attr_name)
{
	kusp_attr *attr;
	
	if (node->attributes == NULL) {
		return NULL;
	}

	HASH_FIND_STR(node->attributes, attr_name, attr);

	return attr;
 }


void kusp_dump_config(kusp_config *config, char *filename)
{
	xmlDocPtr doc;
	xmlNodePtr root_node = NULL;	
	kusp_attr *temp_attr = NULL;	

	if (config == NULL) {
		return;
	}

	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST config->name);


	if (config->attributes != NULL)	{
		for (temp_attr = config->attributes; temp_attr != NULL; 
		                temp_attr = temp_attr->hh.next) {

			xmlNewProp(root_node, BAD_CAST temp_attr->name, 
				  BAD_CAST temp_attr->value);
		}
	}
#ifdef XHASHCONF_DEBUG
	printf("before children\n");
#endif	/* XHASHCONF_DEBUG */
	if (config->children != NULL) {
#ifdef XHASHCONF_DEBUG
		printf("decending\n");
#endif	/* XHASHCONF_DEBUG */
		root_node = build_xml_config(config->children, root_node);
	}

	xmlDocSetRootElement(doc, root_node);

	xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
	xmlFreeDoc(doc);

	xmlCleanupParser();
}

void kusp_free_config(kusp_config *config)
{
	kusp_config *current, *temp, *inner, *inner_temp;
	kusp_attr *attr_current, *attr_temp;

	HASH_ITER(hh, config, current, temp) {

		DL_FOREACH_SAFE(current, inner, inner_temp) {
			if (inner->children != NULL) {
				kusp_free_config(inner->children);
			}
			if (inner->attributes != NULL) {
				HASH_ITER(hh, inner->attributes, attr_current, attr_temp) {
					HASH_DEL(inner->attributes, attr_current);
					free(attr_current);
				}
			}
			
			DL_DELETE(current, inner);
			if (inner != config) {
				free(inner);
				inner = NULL;
			}
		}

		if (current != NULL && current != config) {
			HASH_DEL(config, current);
			free(current);
			current = NULL;
		}
	}

	/* a side effect of HASH_CLEAR is that config is set to null, but we */
	/* still havn't cleaned up config yet so we keep a pointer
	 * around to it 
	 */
	temp = config;
	HASH_CLEAR(hh, config);
	free(temp);
}

static kusp_config* traverse_config(xmlNode *node)
{
	struct _xmlAttr *ptr = NULL;
	xmlNode *cur = NULL;
	xmlChar *xChar = NULL;

	kusp_config *root_config = NULL, *config = NULL;
	kusp_attr *attr = NULL;

	for (cur = node; cur; cur = cur->next) {
		if (cur->type != XML_ELEMENT_NODE) {
			/* uninterested in the non element nodes */
			continue;
		}

		config = alloc_config(cur);
		for (ptr = cur->properties; ptr; ptr = ptr->next) {
			attr = malloc(sizeof(kusp_attr));
			strncpy(attr->name, (const char *) ptr->name, sizeof(attr->name));
			
			xChar = xmlGetProp(cur, ptr->name);
			strncpy(attr->value,(const char *) xChar, sizeof(attr->value));
			xmlFree(xChar);

			/* add attribute to the hash table */
			HASH_ADD_STR(config->attributes, name, attr);
		}

		config->children = traverse_config(cur->children);
		root_config = add_config(root_config, config);
	}

	return root_config;
}

static kusp_config* alloc_config(xmlNode *node)
{
	kusp_config *cur = NULL;

	cur = malloc(sizeof(kusp_config));

	strncpy(cur->name, (const char*)node->name, sizeof(cur->name));
	cur->attributes = NULL;


	cur->content = NULL;
	if (node->content != NULL) {
		cur->content = malloc(sizeof(char) * strlen((const char*)node->content) + 1);
		strcpy(cur->content, (const char*) node->content);
	}

	cur->children = NULL;
	
	cur->next = NULL;
	cur->prev = NULL;

	return cur;
}

static kusp_config* add_config(kusp_config *root, kusp_config *child)
{
	kusp_config *temp;
	HASH_FIND_STR(root, child->name, temp);
	if (temp) {
		//we already have a config, add this one to the list
		
		if (temp->prev == NULL && temp->next == NULL) {
			//need to setup the linked list for the library
			temp->prev = temp;
			temp->next = NULL;
		}

		DL_APPEND(temp, child);
	} else {
		HASH_ADD_STR(root, name, child);
		child->prev = child;
	}

	return root;
}

static xmlNodePtr build_xml_config(kusp_config *config, xmlNodePtr node)
{
	xmlNodePtr temp_node;
	kusp_config *temp_config, *temp_inner_config;
	kusp_attr *temp_attr;

	for (temp_config = config; temp_config; temp_config = temp_config->hh.next) {

		/* for each name at this level */
		DL_FOREACH(temp_config, temp_inner_config) {

#ifdef XHASHCONF_DEBUG
			printf("new node: %s\n", temp_inner_config->name);
#endif	/* XHASHCONF_DEBUG */
			temp_node = xmlNewChild(node, NULL, 
						BAD_CAST temp_inner_config->name, 
						BAD_CAST temp_inner_config->content);

			if (temp_inner_config->attributes == NULL) {
				continue;
			}

			/* for every attribute */
			for (temp_attr = temp_inner_config->attributes; temp_attr; 
			             temp_attr = temp_attr->hh.next) {

				xmlNewProp(temp_node, BAD_CAST temp_attr->name, 
					   BAD_CAST temp_attr->value);
			}

			if (temp_inner_config->children != NULL) {
				temp_node = build_xml_config(temp_inner_config, temp_node);
			}
		}
	}

	return node;
}

#endif	/* XHASHCONF_C */
