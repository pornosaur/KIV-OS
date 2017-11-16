#pragma once

#include <string>
#include <map>
#include <cassert>

#include "FS.h"
#include "FileHandler.h"


class Vfs
{
public:

	Vfs();
	~Vfs();

	uint16_t create_dir(FileHandler **directory, const std::string &absolute_path);
	uint16_t remove_emtpy_dir(const std::string &absolute_path);
	uint16_t open_object(FileHandler **object, const std::string &absolute_path, unsigned int type);
	uint16_t create_file(FileHandler **file, const std::string &absolute_path);
	uint16_t remove_file(const std::string &absolute_path);
	
	uint16_t register_fs(const std::string &name, FS * fs);


private:
	std::map<std::string, FS*> file_systems;

	FS * Vfs::find_fs_by_name(const std::string &name);

	uint16_t translate_return_codes(int fs_ret_code);
};

