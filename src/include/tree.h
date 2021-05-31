/*simple tree functions for storing file info*/
#ifndef _TREE_H_
#include <stdint.h>

#define NODE_TYPE_FILE_FAT 0
#define NODE_TYPE_DIR 1
#define NODE_TYPE_FILE_VIRT 2
#define NODE_MAX_NAME_LENGTH 128	//is limited by data structures, not arbitrary, includes terminating null

struct _tree_node
{
	char name[NODE_MAX_NAME_LENGTH];
	uint16_t id;
	char *virtFileData;
	uint32_t virtFileSize;
	int type;
	struct _tree_node *children;
	struct _tree_node *next;
};
typedef struct _tree_node tree_node;

extern tree_node *tree_find_node(tree_node *root, const char *path);	//null on  failure, never end path with a /, it will search for a file named ""
extern int tree_add_node(tree_node *root, const char *path, tree_node *newnode);	//returns 1 on success
extern void free_entire_tree(tree_node *root);	//if you didn't use malloc, you die

#define _TREE_H_
#endif
