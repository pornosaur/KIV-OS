#pragma once

extern "C" {
	#include "FAT\fat.h"
	#include "FAT/fat_structure.h"
}
#include "Vfs.h"

class VfsFat : public Vfs
{

public:
	VfsFat();
	~VfsFat();

protected:
	struct Vfs::file *create_dir(std::string absolute_path) = 0;
	int remove_emtpy_dir(struct Vfs::file *file) = 0;
	int read_dir(struct Vfs::file *file) = 0;

	struct Vfs::file *open_file(std::string absolute_path) = 0;
	struct Vfs::file *create_file(std::string absolute_path) = 0; /* smaze jiz existujici soubor, existuje-li*/
	int write_to_file(struct Vfs::file *file, char *buffer, int buffer_size) = 0;
	int read_file(struct Vfs::file *file, char *buffer, int buffer_size) = 0;
	int remove_file(struct Vfs::file *file) = 0;

	int close_file(struct Vfs::file *file) = 0;

	int set_file_position(struct Vfs::file *file) = 0;
	int get_file_position(struct Vfs::file *file) = 0;


	int get_current_dir() = 0;
	int set_current_dir() = 0;

private:
	struct Vfs::dentry *VfsFat::find_object_in_directory(struct Vfs::dentry *mDentry, const std::string& dentry_name);
	struct Vfs::dentry *VfsFat::find_object_by_path(std::string absolute_path, int object_type);
};