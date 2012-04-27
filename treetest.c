#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"

int main(int argc, char **argv)
{
	tree_node *root, *new, *debug;
	
	root = (tree_node *)(malloc(sizeof(tree_node)));
//	root->name = NULL;
	memset(root->name,0,sizeof(root->name));
	root->id = 0xA55A;
	root->type = NODE_TYPE_DIR;
	root->children = NULL;
	root->next = NULL;
	
	new = (tree_node *)(malloc(sizeof(tree_node)));
//	new->name = "testdir";
	memset(root->name,0,sizeof(root->name));
	strncpy(new->name, "testdir", sizeof(root->name) - 1);
	new->id = 0xA55B;
	new->type = NODE_TYPE_DIR;
	new->children = NULL;
	new->next = NULL;
	tree_add_node(root, "/", new);
	
	new = (tree_node *)(malloc(sizeof(tree_node)));
//	new->name = "testfile";
	memset(root->name,0,sizeof(root->name));
	strncpy(new->name, "testfile", sizeof(root->name) - 1);
	new->id = 0xA55C;
	new->type = NODE_TYPE_FILE_FAT;
	new->children = NULL;
	new->next = NULL;
	tree_add_node(root, "/testdir", new);
	
	new = (tree_node *)(malloc(sizeof(tree_node)));
//	new->name = "testfile2";
	memset(root->name,0,sizeof(root->name));
	strncpy(new->name, "testfile2", sizeof(root->name) - 1);
	new->id = 0xA55D;
	new->type = NODE_TYPE_FILE_FAT;
	new->children = NULL;
	new->next = NULL;
	tree_add_node(root, "/testdir", new);
	
	new = (tree_node *)(malloc(sizeof(tree_node)));
//	new->name = "testdir2";
	memset(root->name,0,sizeof(root->name));
	strncpy(new->name, "testdir2", sizeof(root->name) - 1);
	new->id = 0xA55E;
	new->type = NODE_TYPE_DIR;
	new->children = NULL;
	new->next = NULL;
	tree_add_node(root, "/testdir", new);
	
	new = (tree_node *)(malloc(sizeof(tree_node)));
//	new->name = "testfile3";
	memset(root->name,0,sizeof(root->name));
	strncpy(new->name, "testfile3", sizeof(root->name) - 1);
	new->id = 0xA55F;
	new->type = NODE_TYPE_FILE_FAT;
	new->children = NULL;
	new->next = NULL;
	tree_add_node(root, "/testdir/testdir2", new);
	
	debug = tree_find_node(root, "/testdir/testdir2/testfile3");
	debug = tree_find_node(root, "/nonexistant");
	debug = tree_find_node(root, "/testdir/nothere");
	debug = tree_find_node(root, "/testdir/testfile/invalid");
	debug = tree_find_node(root, "/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/dontcrash");
	
	free_entire_tree(root);
	return 0;
}
