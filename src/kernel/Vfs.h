#pragma once

#include<string>
#include<map>

#include "FS.h"


class Vfs
{
public:

	Vfs();
	~Vfs();

	int create_dir(struct FS::file **directory, const std::string absolute_path);
	int remove_emtpy_dir(struct FS::file **file);
	int read_dir(struct FS::file *file);
	int open_object(struct FS::file **object, const std::string absolute_path, unsigned int type);
	int create_file(struct FS::file **file, const std::string absolute_path);
	int write_to_file(struct FS::file *file, size_t *writed_bytes, char *buffer, size_t buffer_size);
	int read_file(struct FS::file *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	int remove_file(struct FS::file **file);
	int close_file(struct FS::file **file);

	void set_file_position(struct FS::file *file, unsigned long position); // TODO move to fileHandler
	unsigned long get_file_position(struct FS::file *file); // TODO move to fileHandler
	
	int register_fs(const std::string name, FS * fs);

	//virtual int get_current_dir() = 0;
	//virtual int set_current_dir() = 0;


private:
	std::map<std::string, FS*> file_systems;

	int sb_remove_file(struct FS::file **file);

	FS * Vfs::find_fs_by_name(const std::string name);
};

