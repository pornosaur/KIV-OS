#pragma once
class Vfs
{

	struct list_head {
		void *first;
	};


	struct super_block {
		unsigned long		s_blocksize;		/* block size in bytes */
		unsigned char		s_blocksize_bits;	/* block size in bits */
		unsigned char		s_dirt;			/* dirty flag */
		unsigned long long	s_maxbytes;			/* max file size */
		struct dentry		*s_root;			/* directory mount point */
		unsigned int		s_count;			/* superblock ref count */
		struct list_head	s_inodes;			/* list of inodes */
		struct list_head	s_dirty;			/* list of dirty inodes */
		struct list_head	s_files;			/* list of assigned files */
		char				s_id[32];			/* text name */
	};

	struct inode {
		struct list_head	i_list;			/* list of inodes */
		struct super_block	*i_sb;			/* super block */
		struct list_head	i_dentry;		/* list of dentries */
		unsigned long		i_ino;			/* inode number */
		unsigned int 		i_count;		/* reference counter */
		unsigned long		i_size;			/* file size in bytes */
		unsigned int		i_blkbits;		/* block size in bits */
		unsigned int		i_blocks;		/* file size in blocks */
		unsigned char		i_dirt;			/* dirty flag */
	};

	struct dentry {
		unsigned int 		d_count;		/* reference counter */
		bool				d_mounted;		/* is this a mount point? */
		struct inode		*d_inode;		/* associated inode */
		struct dentry		*d_parent;		/* dentry objects of parent */
		char				d_name[32];		/* dentry name */
		struct list_head	d_subdirs;		/* subdirectories */
		struct super_block	*d_sb;			/* super block */
	};

	struct file {
		struct list_head	f_list;			/* list of file objects */
		struct dentry		*f_dentry;		/* associated dentry */
		unsigned int		d_count;		/* reference counter */
	};

public:
	Vfs();
	~Vfs();
};

