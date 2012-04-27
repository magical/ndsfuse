/*simple tree functions for storing file info*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"

tree_node *tree_find_node(tree_node *root, const char *name)
{
	char buf[NODE_MAX_NAME_LENGTH];
	int i;
	int found;
	size_t len;
	const char *tmp = name;
	tree_node *currentnode = root;
	if(strcmp(name,"/") == 0) return root;

	len = strlen(name);
	while(tmp < (name + len))
	{
		currentnode = currentnode->children;
		if(currentnode == NULL) return NULL;
		if(tmp[0] == '/') tmp++;
		i = 0;
		while(tmp[i] != '/' && tmp[i] != '\x00' && i < (sizeof(buf) - 1))
		{
			buf[i] = tmp[i];
			i++;
		}
		buf[i] = '\x00';
		tmp += i;
		if(tmp[0] != '/' && tmp[0] != '\x00') return NULL;	//we had something too long
		//now look for it
		found = 0;
		do
		{
			if(currentnode == NULL) break;
			if(!strcmp(currentnode->name,buf))
			{
				found = 1;
				break;
			}
			currentnode = currentnode->next;
		}
		while(!found);
		if(!found) return NULL;
	}
	return currentnode;
}

int tree_add_node(tree_node *root, const char *path, tree_node *newnode)
{
	tree_node *addTo = tree_find_node(root, path);
	if(addTo->type != NODE_TYPE_DIR) return 0;
	if(addTo->children == NULL)
	{
		addTo->children = newnode;
		return 1;
	}
	else
	{
		addTo = addTo->children;
		while(addTo->next != NULL)
			addTo = addTo->next;
		addTo->next = newnode;
		return 1;
	}
}

void free_entire_tree(tree_node *root)
{
	if(root == NULL) return;
	free_entire_tree(root->children);
	free_entire_tree(root->next);
	if(root->type == NODE_TYPE_FILE_VIRT && root->virtFileData != NULL) free(root->virtFileData);
	free(root);
}
