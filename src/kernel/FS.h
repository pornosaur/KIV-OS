#pragma once

#include<string>

class FS
{
public:
	struct super_block;
	struct dentry;
	struct file;

	struct super_block {
		unsigned long			s_blocksize;		/* block size in bytes */
		bool					s_dirt;				/* dirty flag */
		unsigned long			s_maxbytes;			/* max file size */
		struct FS::dentry		*s_root;			/* directory mount point */
		unsigned int			s_count;			/* superblock ref count */
		std::string				s_id;				/* text name */
		struct super_block		*s_next;			/* next mounted disk */
	};

	struct dentry {
		FS							*d_fs;				/* pointer to instance of associated FS*/
		struct FS::dentry			*d_parent;			/* dentry objects of parent */

		std::string					d_name;				/* dentry name */
		unsigned int 				d_count;			/* reference counter */
		bool						d_mounted;			/* is this a mount point? */
		unsigned long				d_position;			/* file position (cluster) */
		unsigned long				d_dentry_position;	/* dir entry position (record in parent dir) */
		unsigned int				d_file_type;		/* dir = 0, file = 1*/
		unsigned long				d_size;				/* file size in bytes */
		unsigned long				d_blocks;			/* file size in blocks */
		bool						d_dirt;				/* dirty flag */
		struct FS::dentry			*d_subdirectories;	/* list of open sudirectories */
		struct FS::dentry			*d_next_subdir;		/* next onpen dentry in the same folder */
	};

	struct file {
		struct FS::dentry		*f_dentry;		/* associated dentry */
		unsigned int			f_count;		/* reference counter */
		unsigned long			position;		/* position in file */
	};

	FS();
	~FS();

	// FILE TYPE CONSTANTS
	static const int FS_OBJECT_DIRECTORY = 0;
	static const int FS_OBJECT_FILE = 1;

	// ERROR CONSTANTS
	static const int ERR_SUCCESS = 0;
	static const int ERR_FILE_NOT_FOUND = -1;
	static const int ERR_INVALID_PATH = -2;
	static const int ERR_DIRECTORY_IS_NOT_EMPTY = -3;
	static const int ERR_DIRECTORY_IS_FULL = -4;
	static const int ERR_DISK_IS_FULL = -5;
	static const int ERR_FILE_OPEN_BY_OTHER = -6; // should mean also direcotry had some subdirecotries
	static const int ERR_INVALID_ARGUMENTS = -7;
	static const int ERR_DISK_ERROR = -8;
	static const int ERR_FS_EXISTS = -9;
	static const int ERR_OUT_OF_MEMORY = -10;

	// METHODS FOR WORK WITH FILES
	virtual int fs_create_dir(struct FS::file **directory, const std::string absolute_path) = 0;
	virtual int fs_remove_emtpy_dir(struct FS::file **file) = 0;
	virtual int fs_read_dir(struct FS::file *file) = 0;
	virtual int fs_open_object(struct FS::file **object, const std::string absolute_path, unsigned int type) = 0;
	virtual int fs_create_file(struct FS::file **file, const std::string absolute_path) = 0;
	virtual int fs_write_to_file(struct FS::file *file, size_t *writed_bytes, char *buffer, size_t buffer_size) = 0;
	virtual int fs_read_file(struct FS::file *file, size_t *read_bytes, char *buffer, size_t buffer_size) = 0;
	virtual int fs_remove_file(struct FS::file **file) = 0;
	virtual int fs_close_file(struct FS::file **file) = 0;

	void set_file_position(struct FS::file *file, unsigned long position); // TODO move to fileHandler
	unsigned long get_file_position(struct FS::file *file); // TODO move to fileHandler

	int sb_remove_dentry(struct FS::dentry * dentry); // TODO maybe protected in future

protected:
	struct FS::super_block *sb = NULL;

	struct FS::super_block *FS::init_super_block(
		unsigned long s_blocksize,
		bool s_dirt,
		unsigned long s_maxbytes,
		struct FS::dentry *s_root,
		unsigned int s_count,
		const std::string s_id);

	struct FS::dentry* init_dentry(
		struct FS::dentry *d_parent,
		const std::string d_name,
		unsigned long d_position,
		unsigned long d_dentry_position,
		unsigned int d_file_type,
		unsigned long d_size,
		unsigned long d_blocks);

	struct FS::file *init_file(
		struct FS::dentry *f_dentry,
		unsigned int f_count,
		unsigned long position);

	void FS::sb_remove_all_dentry(struct FS::dentry **d_entry);

	struct FS::dentry *sb_find_dentry_in_dentry(struct FS::dentry * dentry, const std::string name, unsigned int file_type);
};