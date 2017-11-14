#pragma once
#include <string>

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
	FatFS::FatFS(char *memory, size_t memory_size, const std::string &disk_id);
	~FatFS();

	int fs_create_dir(FileHandler **directory, const std::string &absolute_path);
	int fs_remove_emtpy_dir(FileHandler *file);
	int fs_read_dir(FileHandler *file);
	int fs_open_object(FileHandler **object, const std::string &absolute_path, unsigned int type);
	int fs_create_file(FileHandler **file, const std::string &absolute_path); /* smaze jiz existujici soubor, existuje-li*/
	int fs_write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size);
	int fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	int fs_remove_file(FileHandler *file);

	static int FatFS::init_fat_disk(char *memory, size_t memory_size, uint16_t cluster_size);

private:
	struct dentry *FatFS::find_object_in_directory(struct dentry *m_dentry, const std::string& dentry_name, unsigned int type);

	struct dentry *FatFS::find_path(const std::string &absolute_path, size_t *start, size_t *end);
};