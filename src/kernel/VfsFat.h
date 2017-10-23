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

protected:

	struct Vfs::file *create_dir(std::string absolute_path);
	int remove_emtpy_dir(struct Vfs::file *file);
	int read_dir(struct Vfs::file *file);

	struct Vfs::file *open_file(std::string absolute_path);
	struct Vfs::file *create_file(std::string absolute_path); /* smaze jiz existujici soubor, existuje-li*/
	int write_to_file(struct Vfs::file *file, char *buffer, int buffer_size);
	int read_file(struct Vfs::file *file, char *buffer, int buffer_size);
	int remove_file(struct Vfs::file *file);

	int close_file(struct Vfs::file *file);

	//int set_file_position(struct Vfs::file *file);
	//int get_file_position(struct Vfs::file *file);


private:
	struct Vfs::dentry *VfsFat::find_object_in_directory(struct Vfs::dentry *mDentry, const std::string& dentry_name);
};