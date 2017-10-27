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

	Vfs();
	~Vfs();

	virtual struct Vfs::file *create_dir(std::string absolute_path) = 0;
	virtual int remove_emtpy_dir(struct Vfs::file **file) = 0;
	virtual int read_dir(struct Vfs::file *file) = 0;
	
	virtual struct Vfs::file *open_object(std::string absolute_path, int type) = 0;
	virtual struct Vfs::file *create_file(std::string absolute_path) = 0; /* smaze jiz existujici soubor, existuje-li*/
	virtual int write_to_file(struct Vfs::file *file, char *buffer, int buffer_size) = 0;
	virtual int read_file(struct Vfs::file *file, char *buffer, int buffer_size) = 0;
	virtual int remove_file(struct Vfs::file **file) = 0;

	virtual int close_file(struct Vfs::file **file) = 0;

	//virtual int set_file_position(struct Vfs::file *file) = 0;
	//virtual int get_file_position(struct Vfs::file *file) = 0;
	

	//virtual int get_current_dir() = 0;
	//virtual int set_current_dir() = 0;


protected:
	struct Vfs::super_block sb; // TODO kdyz je static nejde prelozit (linker)
	struct Vfs::file root_file;

	void init_super_block(
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
		unsigned int d_count,
		bool d_mounted,
		unsigned long d_position,
		unsigned long d_file_position,
		unsigned int d_file_type,
		unsigned long d_size,
		unsigned int d_blocks,
		unsigned char d_dirt,
		struct Vfs::dentry *d_subdirectories,
		struct Vfs::dentry *d_next_subdir);

	struct Vfs::file *init_file(
		struct Vfs::dentry *f_dentry,
		unsigned int f_count,
		unsigned long position);


	int sb_remove_file(struct Vfs::file **file);

	int sb_remove_dentry(struct Vfs::dentry * dentry);

	struct Vfs::dentry *sb_find_dentry_in_dentry(struct Vfs::dentry * dentry, std::string name);
};

