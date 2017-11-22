#pragma once

#include <string>

#include "VfsStructures.h"
#undef stdin
#undef stdout
#undef stderr
#include "../api/api.h"

#include<mutex>

#define delimeter "\\"
#define delimeter_size 1

class FileHandler;

/**
 * Abstract class for all file systems
 */
class FS
{
public:

	FS() {};
	/** Remove all created dentry which wasnt corectly deleted and delete super_block sb */
	virtual ~FS();

	std::mutex m_mutex; 

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
	static const int ERR_PERMISSION_DENIED = -11;

	/**
	* Return error code defined by api.h which is associated with FS error codes.
	*
	* @param fs_ret_code error code by FS
	* @return error code defined in api.h
	*/
	static uint16_t translate_return_codes(int fs_ret_code);

	// METHODS FOR WORK WITH FILES
	virtual int fs_create_dir(FileHandler **directory, const std::string &absolute_path) = 0;
	virtual int fs_remove_emtpy_dir(FileHandler *file) = 0;
	virtual int fs_read_dir(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size) = 0;
	virtual int fs_open_object(FileHandler **object, const std::string &absolute_path, unsigned int type) = 0;
	virtual int fs_create_file(FileHandler **file, const std::string &absolute_path) = 0;
	virtual int fs_write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size) = 0;
	virtual int fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size) = 0;
	virtual int fs_remove_file(FileHandler *file) = 0;
	virtual int fs_set_file_size(FileHandler *file, size_t file_size) = 0;

	/**
	 * Remove dentry from structure of all dentries
	 *
	 * @param m_dentry dentry which will be removed
	 * @return 0 on success and -1 on error
	 */
	int sb_remove_dentry(struct dentry * dentry);

protected:

	/* Super block asociated to FS*/
	struct super_block *sb = NULL;

	/**
	 * Initialize super block by input parameters and set to variable sb
	 *
	 * @param s_blocksize  block size in bytes
	 * @param s_dirt dirty flag
	 * @param s_maxbytes max file size
	 * @param s_root directory mount point
	 * @param s_count superblock ref count
	 * @param s_id text name
	 * @return created super block
	 */
	struct super_block *init_super_block(
		unsigned long s_blocksize,
		bool s_dirt,
		unsigned long s_maxbytes,
		struct dentry *s_root,
		unsigned int s_count,
		const std::string &s_id);

	/**
	 * Initialize and return dentry fill with input data.
	 * if parent is not null is add new dentry to parent and parent subdirecotries are set as next_subdir
	 *
	 * @param d_parent dentry objects of parent
	 * @param d_name dentry name
	 * @param d_position file position (cluster)
	 * @param d_dentry_position dir entry position (record in parent dir)
	 * @param d_file_type dir = 0, file = 1
	 * @param d_size file size in bytes
	 * @param d_blocks file size in blocks
	 * @return created dentry
	 */
	struct dentry* init_dentry(
		struct dentry *d_parent,
		const std::string &d_name,
		unsigned long d_position,
		unsigned long d_dentry_position,
		unsigned int d_file_type,
		unsigned long d_size,
		unsigned long d_blocks);

	/**
	 * Remove input dentry and all subdentries of input dentry
	 *
	 * @param d_dentry which will be deleted
	 */
	void FS::sb_remove_all_dentry(struct dentry **d_entry);

	/**
	 * Searching for file with name in f_dentry. Searching only in open dentry not in FAT disk
	 *
	 * @param f_dentry file where will be looking for file
	 * @param name name of searching file
	 * @param file_type type of searching file
	 * @return found dentry or NULL
	 */
	struct dentry *sb_find_dentry_in_dentry(struct dentry * dentry, const std::string &name, unsigned int file_type);
};