#pragma once

#include <string>
#include <map>
#include <cassert>

#include "FS.h"
#include "FileHandler.h"

/**
 * Virtual file system create connection to file systems
 */
class Vfs
{
public:

	Vfs() {};
	
	/** Remove all registered file_systems */
	~Vfs();

	/**
	 * By absolute_path find file_system and create directory. If directory exits, will be only opened.
	 *
	 * @param directory File handler representing created directory
	 * @param absolute_path of created directory
	 * @return error codes which are defined in api.h
	 */
	uint16_t create_dir(FileHandler **directory, const std::string &absolute_path);

	/**
	 * Remove empty directory. If directory contains data, error will be returned.
	 *
 	 * @param absolute_path of file which will be removed
	 * @return error codes which are defined in api.h
	 */
	uint16_t remove_emtpy_dir(const std::string &absolute_path);
	
	/**
	 * Open file or directory. Load data about file and returned it in object.
	 *
	 * @param object File handler wich will contains opended file
	 * @param absolute_path to opening file
	 * @param type of opening file (FS_OBJECT_DIRECTORY, FS_OBJECT_FILE);
	 * @return error codes which are defined in api.h
	 */
	uint16_t open_object(FileHandler **object, const std::string &absolute_path, unsigned int type);
	
	/**
	 * By absolute_path find file_system and create file. If file exist, will be deleted.
	 * This function not create directory.
	 *
	 * @param file File handler representing created file
	 * @param absolute_path of created directory
	 * @return error codes which are defined in api.h
	 */
	uint16_t create_file(FileHandler **file, const std::string &absolute_path);
	
	/**
	 * Remove file not directory.
	 *
	 * @param absolute_path to file which will be deleted
	 * @return error codes which are defined in api.h
	 */
	uint16_t remove_file(const std::string &absolute_path);
	
	/**
	 * Registr new file system to VFS. New file system must have unique name.
	 *
	 * @param name unique name of new file system (C:)
	 * @param fs new file system
	 * @return error codes which are defined in api.h
	 */
	uint16_t register_fs(const std::string &name, FS * fs);

private:
	
	/* contains all registered file systems*/
	std::map<std::string, FS*> file_systems;

	/**
	 * Find file system in registered file systems by name
	 *
	 * @param name of searching file system
	 * @return found file system or NULL when file system was not find.
	 */
	FS * Vfs::find_fs_by_name(const std::string &name);
};
