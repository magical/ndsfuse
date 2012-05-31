#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "ndsrom.h"
#include "tree.h"

#define PERMISSION_RXRXRX	S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH
#define PERMISSION_RRR		S_IRUSR | S_IRGRP | S_IROTH

int ndsfuse_getattr(const char *path, struct stat *stbuf)
{
	nds_file *ndsfile;
	tree_node *node;
	memset(stbuf, 0, sizeof(struct stat));
	ndsfile = (nds_file *)(fuse_get_context()->private_data);
	node = tree_find_node(ndsfile->filetree, path);
	if(node == NULL) return -ENOENT;
	stbuf->st_nlink = 1; //FIXME: this is wrong, see FUSE FAQ Why doesn't find work on my filesystem?
	stbuf->st_mtime = ndsfile->mtime;
	stbuf->st_ctime = ndsfile->mtime;
	stbuf->st_atime = ndsfile->mtime;

	if(node->type == NODE_TYPE_DIR)
	{
		stbuf->st_mode = S_IFDIR | PERMISSION_RXRXRX;
	}
	else if(node->type == NODE_TYPE_FILE_VIRT)
	{
		stbuf->st_mode = S_IFREG | PERMISSION_RRR;
		stbuf->st_size = node->virtFileSize;
	}
	else if(node->type == NODE_TYPE_FILE_FAT)
	{
		stbuf->st_mode = S_IFREG | PERMISSION_RRR;
		if((node->id*2+1) < (ndsfile->fatsize/4))
			stbuf->st_size = ndsfile->fat[node->id*2+1] - ndsfile->fat[node->id*2];
		else return -ENOENT;	//fixme
	}
	else return -ENOENT;
	return 0;
}

int ndsfuse_opendir(const char *path, struct fuse_file_info *fi)
{
	nds_file *ndsfile;
	tree_node *node;
	ndsfile = (nds_file *)(fuse_get_context()->private_data);
	node = tree_find_node(ndsfile->filetree, path);
	if(node == NULL) return -ENOENT;
	if(node->type != NODE_TYPE_DIR) return -ENOTDIR;
	return 0;
}

int ndsfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	nds_file *ndsfile;
	tree_node *node;
	ndsfile = (nds_file *)(fuse_get_context()->private_data);
	node = tree_find_node(ndsfile->filetree, path);
	if(node == NULL) return -ENOENT;
	if(node->type != NODE_TYPE_DIR) return -ENOTDIR;
	
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	node = node->children;
	while(node != NULL)
	{
		filler(buf, node->name, NULL, 0);
		node = node->next;
	}
	return 0;
}

int ndsfuse_open(const char *path, struct fuse_file_info *fi)
{
	nds_file *ndsfile;
	tree_node *node;
	ndsfile = (nds_file *)(fuse_get_context()->private_data);
	node = tree_find_node(ndsfile->filetree, path);
	if (fi->flags & O_WRONLY)
		return -EROFS;
	if(node == NULL) return -ENOENT;
	if(node->type == NODE_TYPE_DIR) return -EISDIR;
	
	return 0;
}

static int ndsfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	size_t len;
	nds_file *ndsfile;
	tree_node *node;
	ndsfile = (nds_file *)(fuse_get_context()->private_data);
	node = tree_find_node(ndsfile->filetree, path);
	if (fi->flags & O_WRONLY)
		return -EROFS;
	if(node == NULL) return -ENOENT;
	if(node->type == NODE_TYPE_DIR) return -EISDIR;
	
	if(node->type == NODE_TYPE_FILE_FAT && ((node->id*2+1) < (ndsfile->fatsize/4))) len = ndsfile->fat[node->id*2+1];
	else if(node->type == NODE_TYPE_FILE_VIRT) len = node->virtFileSize;
	else return -ENOENT;	//fixme
	
	if(offset < len)
	{
		if(offset + size > len)
			size = len - offset;
		if(node->type == NODE_TYPE_FILE_FAT)
		{
			size = pread(ndsfile->fd, buf, size, ndsfile->fat[node->id*2]+offset);	//fixme: debug this possibly
		}
		else if(node->type == NODE_TYPE_FILE_VIRT)
		{
			memcpy(buf, node->virtFileData+offset, size);
		}
	}
	else size = 0;
	return size;
}

struct fuse_operations fuse_ops =
{
	.getattr	= ndsfuse_getattr,
	.opendir	= ndsfuse_opendir,
	.readdir	= ndsfuse_readdir,
	.open		= ndsfuse_open,
	.read		= ndsfuse_read,
};

int main(int argc, char **argv)
{
	nds_file *ndsfile;
	struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
	int i, ret;
	char *filename;

	if(argc < 3)
	{
		printf("Usage: %s file.nds mount_point [fuse_options]\n",argv[0]);
		return 1;
	}
	
	filename = argv[1];
	ndsfile = nds_do_magic(filename);
	
	if(ndsfile == NULL)
	{
		printf("Some error occured!\n");
		return 1;
	}
	
	for(i=0;i<argc;i++)
	{
		if(i != 1) fuse_opt_add_arg(&args, argv[i]);
	}
	
	ret = fuse_main(args.argc, args.argv, &fuse_ops, ndsfile);
	
	fuse_opt_free_args(&args);
	free_nds_file(ndsfile);
	
	return ret;
}
