#pragma once

#include<string>

class Vfs
{
public:
	struct super_block;
	struct dentry;
	struct file;

	struct super_block {
		unsigned long			s_blocksize;		/* block size in bytes */
		bool					s_dirt;				/* dirty flag */
		unsigned long long		s_maxbytes;			/* max file size */
		struct Vfs::dentry		*s_root;			/* directory mount point */
		unsigned int			s_count;			/* superblock ref count */
		std::string				s_id;				/* text name */
		struct super_block		*s_next;			/* next mounted disk */
	};

	struct dentry {
		struct Vfs::super_block		*d_sb;				/* super block */
		struct Vfs::dentry			*d_parent;			/* dentry objects of parent */

		std::string					d_name;				/* dentry name */
		unsigned int 				d_count;			/* reference counter */
		bool						d_mounted;			/* is this a mount point? */
		unsigned long				d_position;			/* file position (cluster) */
		unsigned long				d_file_position;	/* dir entry position (record in parent dir) */
		unsigned int				d_file_type;		/* dir = 0, file = 1*/
		unsigned long				d_size;				/* file size in bytes */
		unsigned int				d_blocks;			/* file size in blocks */
		unsigned char				d_dirt;				/* dirty flag */
		struct Vfs::dentry			*d_subdirectories;	/* list of open sudirectories */
		struct Vfs::dentry			*d_next_subdir;		/* next onpen dentry in the same folder */
	};

	struct file {
		struct Vfs::dentry		*f_dentry;		/* associated dentry */
		unsigned int			f_count;		/* reference counter */
		unsigned long			position;		/* position in file */
	};

	static const int VFS_OBJECT_DIRECTORY = 0;
	static const int VFS_OBJECT_FILE = 1;

	static const int ERR_SUCCESS = 0;

	static const int ERR_FILE_NOT_FOUND = -1;
	static const int ERR_INVALID_PATH = -2;
	static const int ERR_DIRECTORY_IS_NOT_EMPTY = -3;
	static const int ERR_DIRECTORY_IS_FULL = -4;
	static const int ERR_DISK_IS_FULL = -5;
	static const int ERR_FILE_OPEN_BY_OTHER = -6; // should mean also direcotry had some subdirecotries
	static const int ERR_INVALID_ARGUMENTS = -7;
	static const int ERR_DISK_ERROR = -8;

	Vfs();
	~Vfs();

	virtual int create_dir(struct Vfs::file **directory, std::string absolute_path) = 0;
	virtual int remove_emtpy_dir(struct Vfs::file **file) = 0;
	virtual int read_dir(struct Vfs::file *file) = 0;
	
	virtual int open_object(struct Vfs::file **object, std::string absolute_path, int type) = 0;
	virtual int create_file(struct Vfs::file **file, std::string absolute_path) = 0;
	virtual int write_to_file(struct Vfs::file *file, char *buffer, int buffer_size) = 0;
	virtual int read_file(struct Vfs::file *file, char *buffer, int buffer_size) = 0;
	virtual int remove_file(struct Vfs::file **file) = 0;

	virtual int close_file(struct Vfs::file **file) = 0;

	void set_file_position(struct Vfs::file *file, unsigned long position);
	unsigned long get_file_position(struct Vfs::file *file);
	

	//virtual int get_current_dir() = 0;
	//virtual int set_current_dir() = 0;


protected:
	struct Vfs::super_block *sb = NULL;

	struct Vfs::super_block *Vfs::init_super_block(
		unsigned long s_blocksize,
		bool s_dirt,
		unsigned long long s_maxbytes,
		struct Vfs::dentry *s_root,
		unsigned int s_count,
		std::string s_id);

	struct Vfs::dentry* init_dentry(
		struct Vfs::super_block *d_sb,
		struct Vfs::dentry *d_parent,
		std::string d_name,
		unsigned long d_position,
		unsigned long d_file_position,
		unsigned int d_file_type,
		unsigned long d_size,
		unsigned int d_blocks);

	struct Vfs::file *init_file(
		struct Vfs::dentry *f_dentry,
		unsigned int f_count,
		unsigned long position);

	struct Vfs::super_block *find_super_block_by_name(std::string name);

	int sb_remove_file(struct Vfs::file **file);

	int sb_remove_dentry(struct Vfs::dentry * dentry);

	void Vfs::sb_remove_all_dentry(struct Vfs::dentry **d_entry);

	struct Vfs::dentry *sb_find_dentry_in_dentry(struct Vfs::dentry * dentry, std::string name, int file_type);
};

