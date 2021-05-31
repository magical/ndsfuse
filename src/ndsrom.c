/*nds parsing stuff*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "ndsrom.h"
#include "tree.h"

#define DIR_ID_BASE	0xF000
#define ROOT_DIR_ID	DIR_ID_BASE

#define SUBTABLE_END		0x00
#define SUBTABLE_FILE_MIN	0x01
#define SUBTABLE_FILE_MAX	0x7F
#define SUBTABLE_DIR_BASE	0x80	//lengths for subdirectories have this added
#define SUBTABLE_DIR_MIN	0x81
#define SUBTABLE_DIR_MAX	0xFF

static int nds_do_fnt_entry(nds_fnt_main_table_entry *entry, uint8_t *fnt, tree_node *addhere)	//returns 1 if it worked
{
	uint8_t *subtabletmp;
	tree_node *tmpnode;
	uint16_t fileid = entry->firstfileid;
	subtabletmp = fnt+entry->subtableoff;
	while(subtabletmp[0] != SUBTABLE_END)
	{
		tmpnode = (tree_node *)(malloc(sizeof(tree_node)));
		if(tmpnode == NULL) return 0;
		memset(tmpnode->name, 0, sizeof(tmpnode->name));
		tmpnode->children = NULL;
		tmpnode->next = NULL;
		if(subtabletmp[0] >= SUBTABLE_FILE_MIN && subtabletmp[0] <= SUBTABLE_FILE_MAX)	//file
		{
			tmpnode->type = NODE_TYPE_FILE_FAT;
			tmpnode->id = fileid++;
			strncpy(tmpnode->name, (char *)(subtabletmp+1), subtabletmp[0]);
			tree_add_node(addhere, "/", tmpnode);
			subtabletmp += subtabletmp[0] + 1;	//subtabletmp[0] is name length, +1 length byte
		}
		else if(subtabletmp[0] >= SUBTABLE_DIR_MIN && subtabletmp[0] <= SUBTABLE_DIR_MAX)	//dir
		{
			tmpnode->type = NODE_TYPE_DIR;
			tmpnode->id = *((uint16_t *)(subtabletmp + subtabletmp[0] - SUBTABLE_DIR_BASE + 1));	//this looks up the dir id, which is after the length/type and name
			strncpy(tmpnode->name, (char *)(subtabletmp+1), subtabletmp[0] - SUBTABLE_DIR_BASE);
			tree_add_node(addhere, "/", tmpnode);
			subtabletmp += subtabletmp[0] - SUBTABLE_DIR_BASE + 1 + 2;	//subtabletmp[0] is name length + SUBTABLE_DIR_BASE +1 length byte +2 id
			nds_do_fnt_entry(&(((nds_fnt_main_table_entry *)(fnt))[tmpnode->id - DIR_ID_BASE]), fnt, tmpnode);	//treat tmpnode as the new root
		}
	}
	return 1;
}

static tree_node *nds_prepare_virtual_file(int fd, uint32_t offset, uint32_t size, const char *name)
{
	char *tmp;
	tree_node *tmpnode;
	tmp = (char *)(malloc(size));
	if(tmp == NULL) return NULL;
	if(lseek(fd, offset, SEEK_SET) < 0) return NULL;
	if(read(fd, tmp, size) != size) return NULL;
	tmpnode = (tree_node *)(malloc(sizeof(tree_node)));
	if(tmpnode == NULL) return NULL;
	memset(tmpnode->name, 0, sizeof(tmpnode->name));
	strncpy(tmpnode->name, name, sizeof(tmpnode->name) - 1);
	tmpnode->virtFileData = tmp;
	tmpnode->virtFileSize = size;
	tmpnode->type = NODE_TYPE_FILE_VIRT;
	tmpnode->children = NULL;
	tmpnode->next = NULL;
	return tmpnode;
}

static tree_node *nds_do_overlay(int fd, uint32_t offset, uint32_t size, const char *name)
{
	char *tmp;
	nds_overlay_entry *overlay;
	tree_node *tmpnode, *tmpnode2;

	tmpnode = (tree_node *)(malloc(sizeof(tree_node)));
	if(tmpnode == NULL) return NULL;
	memset(tmpnode->name, 0, sizeof(tmpnode->name));
	strncpy(tmpnode->name, name, sizeof(tmpnode->name) - 1);
	tmpnode->type = NODE_TYPE_DIR;
	tmpnode->children = NULL;
	tmpnode->next = NULL;
	tmp = (char *)(malloc(size));
	if(tmp == NULL) return NULL;
	if(lseek(fd, offset, SEEK_SET) < 0) return NULL;
	if(read(fd, tmp, size) != size) return NULL;
	overlay = (nds_overlay_entry *)(tmp);
	while(overlay < (nds_overlay_entry *)(tmp + size))
	{
		tmpnode2 = (tree_node *)(malloc(sizeof(tree_node)));
		if(tmpnode2 == NULL) return NULL;
		memset(tmpnode2->name, 0, sizeof(tmpnode2->name));
		snprintf(tmpnode2->name, sizeof(tmpnode2->name), "overlay_%08X.bin", overlay->overlayid);
		tmpnode2->type = NODE_TYPE_FILE_FAT;
		tmpnode2->id = overlay->fileid;
		tmpnode2->children = NULL;
		tmpnode2->next = NULL;
		overlay++;
		if(!tree_add_node(tmpnode, "/", tmpnode2)) return NULL;
	}
	return tmpnode;
}

nds_file *nds_do_magic(const char *file)
{
	tree_node *root, *tmpnode;
	int fd;
	nds_header header;
	char *tmp;
	nds_file *ret;
	struct stat stat;
	
	fd = open(file, O_RDONLY);
	if(fd < 0) return NULL;

	if (fstat(fd, &stat) < 0) {
		return NULL;
	}

	ret = (nds_file *)(malloc(sizeof(nds_file)));
	if(ret == NULL) return NULL;

	ret->mtime = stat.st_mtime;

	root = (tree_node *)(malloc(sizeof(tree_node)));
	if(root == NULL) return NULL;
	memset(root->name,0,sizeof(root->name));
	root->type = NODE_TYPE_DIR;
	root->children = NULL;
	root->next = NULL;
	ret->filetree = root;
	ret->fd = fd;

	if(read(fd, &header, sizeof(header)) != sizeof(header)) return NULL;

	//make header.bin
	tmpnode = nds_prepare_virtual_file(fd, /*beginning of file*/0, header.headersize, "header.bin");
	if(tmpnode == NULL) return NULL;
	if(!tree_add_node(root, "/", tmpnode)) return NULL;

	//make arm9.bin
	tmpnode = nds_prepare_virtual_file(fd, header.arm9rom, header.arm9size, "arm9.bin");
	if(tmpnode == NULL) return NULL;
	if(!tree_add_node(root, "/", tmpnode)) return NULL;

	//make arm7.bin
	tmpnode = nds_prepare_virtual_file(fd, header.arm7rom, header.arm7size, "arm7.bin");
	if(tmpnode == NULL) return NULL;
	if(!tree_add_node(root, "/", tmpnode)) return NULL;

	//read fat
	tmp = (char *)(malloc(header.fatsize));
	if(tmp == NULL) return NULL;
	if(lseek(fd, header.fatoffset, SEEK_SET) < 0) return NULL;
	if(read(fd, tmp, header.fatsize) != header.fatsize) return NULL;
	ret->fat = (uint32_t *)(tmp);
	ret->fatsize = header.fatsize;

	//real magic here
	tmpnode = (tree_node *)(malloc(sizeof(tree_node)));
	if(tmpnode == NULL) return NULL;
	memset(tmpnode->name, 0, sizeof(tmpnode->name));
	strncpy(tmpnode->name, "fsroot", sizeof(tmpnode->name) - 1);
	tmpnode->id = ROOT_DIR_ID;
	tmpnode->type = NODE_TYPE_DIR;
	tmpnode->children = NULL;
	tmpnode->next = NULL;
	if(!tree_add_node(root, "/", tmpnode)) return NULL;
	tmp = (char *)(malloc(header.fntsize));
	if(tmp == NULL) return NULL;
	if(lseek(fd, header.fntoffset, SEEK_SET) < 0) return NULL;
	if(read(fd, tmp, header.fntsize) != header.fntsize) return NULL;
	if(nds_do_fnt_entry((nds_fnt_main_table_entry *)(tmp), (uint8_t *)tmp, tmpnode) != 1) return NULL;
	free(tmp);
	
	if(header.arm9overlayoffset)
	{
		tmpnode = nds_do_overlay(fd, header.arm9overlayoffset, header.arm9overlaysize, "overlay9");
		if(tmpnode == NULL) return NULL;
		if(!tree_add_node(root, "/", tmpnode)) return NULL;
	}

	if(header.arm7overlayoffset)
	{
		tmpnode = nds_do_overlay(fd, header.arm7overlayoffset, header.arm7overlaysize, "overlay7");
		if(tmpnode == NULL) return NULL;
		if(!tree_add_node(root, "/", tmpnode)) return NULL;
	}
	
	return ret;
}

void free_nds_file(nds_file *file)
{
	close(file->fd);
	free_entire_tree(file->filetree);
	free(file->fat);
	free(file);
}
