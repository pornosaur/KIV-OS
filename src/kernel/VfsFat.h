#include <string>

#include "Vfs.h"

extern "C" {
	#include "FAT/fat.h"
	#include "FAT/fat_structure.h"
}

class VfsFat : public Vfs
{

public:
	VfsFat();
	~VfsFat();

	int create_dir(struct Vfs::file **directory, const std::string absolute_path);
	int remove_emtpy_dir(struct Vfs::file **file);
	int read_dir(struct Vfs::file *file);

	int open_object(struct Vfs::file **object, const std::string absolute_path, unsigned int type);
	int create_file(struct Vfs::file **file, const std::string absolute_path); /* smaze jiz existujici soubor, existuje-li*/
	int write_to_file(struct Vfs::file *file, size_t *writed_bytes, char *buffer, size_t buffer_size);
	int read_file(struct Vfs::file *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	int remove_file(struct Vfs::file **file);

	virtual int close_file(struct Vfs::file **file);


private:
	struct Vfs::dentry *VfsFat::find_object_in_directory(struct Vfs::dentry *m_dentry, const std::string& dentry_name, unsigned int type);

	struct Vfs::dentry *VfsFat::find_path(const std::string absolute_path, size_t *start, size_t *end);
};