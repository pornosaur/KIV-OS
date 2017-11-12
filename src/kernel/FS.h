#pragma once

#include <string>

#include "FileHandler.h"
#include "VfsStructures.h"

class FS
{
public:

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
	virtual int fs_create_dir(FileHandler **directory, const std::string absolute_path) = 0;
	virtual int fs_remove_emtpy_dir(FileHandler **file) = 0;
	virtual int fs_read_dir(FileHandler *file) = 0;
	virtual int fs_open_object(FileHandler **object, const std::string absolute_path, unsigned int type) = 0;
	virtual int fs_create_file(FileHandler **file, const std::string absolute_path) = 0;
	virtual int fs_write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size) = 0;
	virtual int fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size) = 0;
	virtual int fs_remove_file(FileHandler **file) = 0;
	virtual int fs_close_file(FileHandler **file) = 0;

	void set_file_position(FileHandler *file, unsigned long position); // TODO move to fileHandler
	unsigned long get_file_position(FileHandler *file); // TODO move to fileHandler

	int sb_remove_dentry(struct dentry * dentry); // TODO maybe protected in future

protected:
	struct super_block *sb = NULL;

	struct super_block *init_super_block(
		unsigned long s_blocksize,
		bool s_dirt,
		unsigned long s_maxbytes,
		struct dentry *s_root,
		unsigned int s_count,
		const std::string s_id);

	struct dentry* init_dentry(
		struct dentry *d_parent,
		const std::string d_name,
		unsigned long d_position,
		unsigned long d_dentry_position,
		unsigned int d_file_type,
		unsigned long d_size,
		unsigned long d_blocks);

	void FS::sb_remove_all_dentry(struct dentry **d_entry);

	struct dentry *sb_find_dentry_in_dentry(struct dentry * dentry, const std::string name, unsigned int file_type);
};