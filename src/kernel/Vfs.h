#pragma once

#include<string>
#include<map>

#include "FS.h"
#include "FileHandler.h"


class Vfs
{
public:

	Vfs();
	~Vfs();

	int create_dir(FileHandler **directory, const std::string absolute_path);
	int remove_emtpy_dir(FileHandler **file);
	int read_dir(FileHandler *file);
	int open_object(FileHandler **object, const std::string absolute_path, unsigned int type);
	int create_file(FileHandler **file, const std::string absolute_path);
	int write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size);
	int read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	int remove_file(FileHandler **file);
	int close_file(FileHandler **file);

	void set_file_position(FileHandler *file, unsigned long position); // TODO move to fileHandler
	unsigned long get_file_position(FileHandler *file); // TODO move to fileHandler
	
	int register_fs(const std::string name, FS * fs);

	//virtual int get_current_dir() = 0;
	//virtual int set_current_dir() = 0;


private:
	std::map<std::string, FS*> file_systems;

	int sb_remove_file(FileHandler **file);

	FS * Vfs::find_fs_by_name(const std::string name);
};

