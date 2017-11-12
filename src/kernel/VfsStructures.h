#pragma once

#include<string>

class FS;

struct super_block;
struct dentry;

struct super_block {
	unsigned long			s_blocksize;		/* block size in bytes */
	bool					s_dirt;				/* dirty flag */
	unsigned long			s_maxbytes;			/* max file size */
	struct dentry			*s_root;			/* directory mount point */
	unsigned int			s_count;			/* superblock ref count */
	std::string				s_id;				/* text name */
	struct super_block		*s_next;			/* next mounted disk */
};

struct dentry {
	FS							*d_fs;				/* pointer to instance of associated FS*/
	struct dentry				*d_parent;			/* dentry objects of parent */

	std::string					d_name;				/* dentry name */
	unsigned int 				d_count;			/* reference counter */
	bool						d_mounted;			/* is this a mount point? */
	unsigned long				d_position;			/* file position (cluster) */
	unsigned long				d_dentry_position;	/* dir entry position (record in parent dir) */
	unsigned int				d_file_type;		/* dir = 0, file = 1*/
	unsigned long				d_size;				/* file size in bytes */
	unsigned long				d_blocks;			/* file size in blocks */
	bool						d_dirt;				/* dirty flag */
	struct dentry				*d_subdirectories;	/* list of open sudirectories */
	struct dentry				*d_next_subdir;		/* next onpen dentry in the same folder */
};