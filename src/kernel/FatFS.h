#include <string>

#include "FS.h"

extern "C" {
	#include "FAT/fat.h"
	#include "FAT/fat_structure.h"
}

class FatFS : public FS
{

public:
	FatFS(char *memory, size_t memory_size);
	~FatFS();

	int fs_create_dir(struct FS::file **directory, const std::string absolute_path);
	int fs_remove_emtpy_dir(struct FS::file **file);
	int fs_read_dir(struct FS::file *file);
	int fs_open_object(struct FS::file **object, const std::string absolute_path, unsigned int type);
	int fs_create_file(struct FS::file **file, const std::string absolute_path); /* smaze jiz existujici soubor, existuje-li*/
	int fs_write_to_file(struct FS::file *file, size_t *writed_bytes, char *buffer, size_t buffer_size);
	int fs_read_file(struct FS::file *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	int fs_remove_file(struct FS::file **file);
	int fs_close_file(struct FS::file **file);

	static int FatFS::init_fat_disk(char *memory, size_t memory_size, uint16_t cluster_size);

private:
	struct FS::dentry *FatFS::find_object_in_directory(struct FS::dentry *m_dentry, const std::string& dentry_name, unsigned int type);

	struct FS::dentry *FatFS::find_path(const std::string absolute_path, size_t *start, size_t *end);
};