#pragma once
#include <string>
#include <cassert>

#include "VfsStructures.h"
#include "FileHandler.h"
#include "FS.h"

extern "C" {
	#include "FAT/fat.h"
	#include "FAT/fat_structure.h"
}

class FatFS : public FS
{

public:
	
	/**
	 * Initialize FAT disk with input arguments
	 *
	 * @param memory array representing FAT disk
	 * @param memory_size size of memory array
	 * @param name of disk (e.g. C:)
	 */
	FatFS(char *memory, size_t memory_size, const std::string &disk_id);
	
	/** Remove Fat disk and associated data */
	~FatFS();

	/**
	 * Create new directory defined by absolute_path. If direcotry exists, only increment counter and return it.
	 *
	 * @param directory new created directory. Will be overwrite in method.
	 * @param sbsolute_path String representing absolute path to new directory
	 * @return error codes which are defined in FS.h
	 */
	int fs_create_dir(FileHandler **directory, const std::string &absolute_path);
	
	/**
	 * Remove directory by input file. If directory is not empty return ERROR CODE
	 *
	 * @param file file handler representing removing file
	 * @return error codes which are defined in FS.h
	 */
	int fs_remove_emtpy_dir(FileHandler *file);
	
	/**
	 * Read file records in directory and saved it in buffer like TDir_Entry.
	 * Set file position to end of read. When is called read on end of file, read_bytes = 0 and return ERR_SUCCESS
	 *
	 * @param file File handler representing read file
	 * @param read_bytes return number of read bytes
	 * @param buffer for read data
	 * @param buffer_size size of buffer
	 * @return error codes which are defined in FS.h
	 */
	int fs_read_dir(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	
	/**
	 * Open file in FAT disk which type and position is defined in arguments.
	 *
	 * @param object File handler representing opened file
	 * @param absolute_path path and file name of opened file
	 * @param type of file (FS_OBJECT_DIRECTORY, FS_OBJECT_FILE)
	 * @return error codes which are defined in FS.h
	 */
	int fs_open_object(FileHandler **object, const std::string &absolute_path, unsigned int type);
	
	/**
	 * Create file(FS_OBJECT_FILE) in FAT disk on position given by absolute_path.
	 * If file exists, old file is deleted.
	 *
	 * @param file File handler representing created file
	 * @param absolute_path path and file name of created file
	 * @return error codes which are defined in FS.h
	 */
	int fs_create_file(FileHandler **file, const std::string &absolute_path); /* smaze jiz existujici soubor, existuje-li*/
	
	/**
	 * Write to file input buffer.
	 * Set file position to end of writed data.
	 * Writing start at actual file position.
	 *
	 * @param file File handler representing file where will be write data
	 * @param writed_bytes return number of writed bytes
	 * @param buffer contains data to write
	 * @param buffer_size size of buffer
	 * @return error codes which are defined in FS.h
	 */
	int fs_write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size);
	
	/**
	 * Read data from file and save it buffer.
	 * Set file position to end of read data.
	 * Reading start at actual file position.
	 * When is called read on end of file, read_bytes = 0 and return ERR_SUCCESS
	 *
	 * @param file File handler representing file which will be read
	 * @param read_bytes return number of read bytes
	 * @param buffer will be contains read data
	 * @param buffer_size size of buffer
	 * @return error codes which are defined in FS.h
	 */
	int fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	
	/**
	 * Remove file(FS_OBJECT_FILE) from FAT disk
	 *
	 * @param file file handler repressenting file to delete
	 * @return error codes which are defined in FS.h
	 */
	int fs_remove_file(FileHandler *file);
	
	/**
	 * Set file size to given size. file_size must be smaller than old file size.
	 *
	 * @param file File handler which representing file which size will be changed
	 * @param file_size new size of file
	 * @return error codes which are defined in FS.h
	 */
	int fs_set_file_size(FileHandler *file, size_t file_size);

	/**
	 * Format array memory as a FAT disk. From cluster_size and memory_size calculate maximal size of disk.
	 *
	 * @param memory array representing FAT disk
	 * @param memory_size size of memory array
	 * @param size of one cluster in FAT disk
	 * @return ERR_SUCCESS
	 */
	static int init_fat_disk(char *memory, size_t memory_size, uint16_t cluster_size);

private:

	/* Keep data of inicializated FAT disk*/
	struct fat_data *f_data = NULL;

	/**
	 * Find file/directory in directory given by m_dentry.
	 *
	 * @param m_dentry directory where should be searched file
	 * @param dentry_name name of searched file
	 * @param type of file (FS_OBJECT_DIRECTORY, FS_OBJECT_FILE)
	 * @return error codes which are defined in FS.h
	 */
	struct dentry *find_object_in_directory(struct dentry *m_dentry, const std::string& dentry_name, unsigned int type);

	/**
	 * Parse absolute_path and check if files exists. Last file in path is not tested.
	 * Start and end will contains positions for substring of absolute_path for last file.
	 *
	 * @param absolute_path path which is tested
	 * @param start return start index of last file in absolute_path
	 * @param end return end index of last file in absolute_path
	 * @return penultimate file in absolute_path or NULL on error
	 */
	struct dentry *find_path(const std::string &absolute_path, size_t *start, size_t *end);
	
	/**
	 * Create struct dir_file from struct dentry.
	 * struct dentry is used in VFS but struct dir_file is used in FAT
	 *
	 * @param dentry representing file from which will be created dir_file
	 * @return created dir_file
	 */
	struct dir_file *create_dir_file(struct dentry *dentry);
};