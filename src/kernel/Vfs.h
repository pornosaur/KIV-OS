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

	uint16_t create_dir(FileHandler **directory, const std::string &absolute_path);
	uint16_t remove_emtpy_dir(FileHandler **file);
	uint16_t read_dir(FileHandler *file);
	uint16_t open_object(FileHandler **object, const std::string &absolute_path, unsigned int type);
	uint16_t create_file(FileHandler **file, const std::string &absolute_path);
	uint16_t write_to_file(Handler *file, size_t *writed_bytes, char *buffer, size_t buffer_size);
	uint16_t read_file(Handler *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	uint16_t remove_file(FileHandler **file);
	uint16_t close_file(FileHandler **file);

	uint16_t set_file_position(FileHandler * file, long offset, uint8_t origin);
	size_t get_file_position(FileHandler *file);
	
	uint16_t register_fs(const std::string &name, FS * fs);


private:
	std::map<std::string, FS*> file_systems;

	FS * Vfs::find_fs_by_name(const std::string &name);

	int sb_remove_file(FileHandler **file);

	uint16_t translate_return_codes(int fs_ret_code);
};

