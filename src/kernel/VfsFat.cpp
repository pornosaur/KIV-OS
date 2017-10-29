#include "VfsFat.h"

VfsFat::VfsFat()
{
	fat_init("output.fat");
	if (result != 0 || !is_boot_record_init())
	{
		return; // TODO rict systemu ze nelze pracovat s FAT
	}


	struct Vfs::super_block * sb = Vfs::init_super_block(
		get_cluster_size(), 0,
		get_fat_size_in_bytes(),
		NULL, 1, "FAT_DISK");

	struct Vfs::dentry *root = Vfs::init_dentry(sb, NULL, "C:", 1, 1, 0, get_start_of_root_dir(), VFS_OBJECT_DIRECTORY,
		get_dir_size_in_bytes(), get_dir_clusters(), 0, NULL, NULL);

	sb->s_root = root;
}


VfsFat::~VfsFat()
{
	close_fat();

	struct Vfs::super_block *sb = Vfs::sb;
	struct Vfs::super_block *next = NULL;
	Vfs::sb = NULL;

	while (sb != NULL) {
		delete sb->s_root;
		next = sb->s_next;
		delete sb;
		sb = next;
	}
}

struct Vfs::file *VfsFat::create_dir(std::string absolute_path)
{
	struct Vfs::dentry *mDentry = NULL;
	struct Vfs::dentry *fDentry = NULL;
	size_t start = 0;
	size_t end = 0;

	mDentry = VfsFat::find_path(absolute_path, &start, &end);
	if (mDentry == NULL) {
		return NULL;
	}

	if (start == 0 && end == std::string::npos) {
		fDentry = mDentry;
	} 
	else {
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end), VFS_OBJECT_DIRECTORY);
	}

	if (fDentry == NULL) {
		long dir_position = 0;
		struct dir_file *dirFile = fat_create_dir(absolute_path.substr(start, end).c_str(), mDentry->d_position, &dir_position);	
		if (dirFile == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL; // cant create file on physic medium
		}

		fDentry = Vfs::init_dentry(sb, mDentry, absolute_path.substr(start, end), 0, 0, dirFile->first_cluster, dir_position, dirFile->file_type,
			dirFile->file_size, (long) ceil((double)dirFile->file_size / get_cluster_size()), 0, NULL, NULL);
		fDentry->d_next_subdir = mDentry->d_subdirectories;
		mDentry->d_subdirectories = fDentry;
		mDentry->d_count++;
		free(dirFile);
	}

	fDentry->d_count++;
	return Vfs::init_file(fDentry, 0, 0);
}

int VfsFat::remove_emtpy_dir(struct Vfs::file **file)
{
	if (*file == NULL || (*file)->f_dentry == NULL || (*file)->f_dentry->d_parent == NULL || (*file)->f_dentry->d_file_type != Vfs::VFS_OBJECT_DIRECTORY || (*file)->f_dentry->d_count > 1) {
		// TODO mohu vymazat File, protoze f_dentry je NULL
		return -1;
	}

	int result = fat_delete_empty_dir((*file)->f_dentry->d_name.c_str(), (*file)->f_dentry->d_parent->d_position);

	if (result == 0) {
		Vfs::sb_remove_file(file);
	} 
	else
	{
		return -1;
	}

	return 0;
}

int VfsFat::read_dir(struct Vfs::file *file)
{
	return NULL;
	//TODO
}

struct Vfs::file *VfsFat::create_file(std::string absolute_path)
{
	struct Vfs::dentry *mDentry = NULL;
	struct Vfs::dentry *fDentry = NULL;
	size_t start = 0;
	size_t end = 0;

	mDentry = VfsFat::find_path(absolute_path, &start, &end);
	if (mDentry == NULL) {
		return NULL;
	}
	if (start == 0 && end == std::string::npos)
	{
		Vfs::sb_remove_dentry(mDentry);
		return NULL;
	}

	fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end), VFS_OBJECT_FILE);

	if (fDentry != NULL) {
		if (fDentry->d_count > 0) {
			return NULL;
		}
		fat_delete_file_by_name(fDentry->d_name.c_str(), fDentry->d_parent->d_position);
		Vfs::sb_remove_dentry(fDentry);
		fDentry = NULL;
	}

	long dir_position = 0;
	struct dir_file *dirFile = fat_create_file(absolute_path.substr(start, end).c_str(), mDentry->d_position, &dir_position);
	if (dirFile == NULL) {
		Vfs::sb_remove_dentry(mDentry);
		return NULL; // cant create file on physic medium
	}
	
	fDentry = Vfs::init_dentry(sb, mDentry, absolute_path.substr(start, end), 0, 0, dirFile->first_cluster, dir_position, dirFile->file_type,
	dirFile->file_size, (long) std::ceil((double)dirFile->file_size / get_cluster_size()), 0, NULL, NULL);
	fDentry->d_next_subdir = mDentry->d_subdirectories;
	mDentry->d_subdirectories = fDentry;
	mDentry->d_count++;
	free(dirFile);

	fDentry->d_count++;
	return Vfs::init_file(fDentry, 0, 0);
}

struct Vfs::file *VfsFat::open_object(std::string absolute_path, int type)
{
	struct Vfs::dentry *mDentry = NULL;
	struct Vfs::dentry *fDentry = NULL;
	size_t start = 0;
	size_t end = 0;

	mDentry = VfsFat::find_path(absolute_path, &start, &end);
	if (mDentry == NULL) {
		return NULL;
	}

	if (start == 0 && end == std::string::npos){
		if (type != Vfs::VFS_OBJECT_DIRECTORY)
		{
			Vfs::sb_remove_dentry(mDentry);
			return NULL;
		}
		fDentry = mDentry;
	} 
	else
	{
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end), type);
		if (fDentry == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL;
		}
	}
	

	fDentry->d_count++;
	return Vfs::init_file(fDentry, 0, 0);
}

struct Vfs::dentry *VfsFat::find_object_in_directory(struct Vfs::dentry *mDentry, const std::string& dentry_name, int type) {
	
	struct Vfs::dentry *fDentry = Vfs::sb_find_dentry_in_dentry(mDentry, dentry_name, type);

	if (fDentry == NULL) {
		long dir_position = 0;
		struct dir_file *dirFile = fat_get_object_info_by_name(dentry_name.c_str(), type, mDentry->d_position, &dir_position);
		
		if (dirFile == NULL) {
			return NULL;
		}

		fDentry = Vfs::init_dentry(mDentry->d_sb, mDentry, dentry_name, 0, 0, dirFile->first_cluster, dir_position, dirFile->file_type,
			dirFile->file_size, (long)ceil((double)dirFile->file_size / get_cluster_size()), 0, NULL, NULL);
		fDentry->d_next_subdir = mDentry->d_subdirectories;
		mDentry->d_subdirectories = fDentry;
		mDentry->d_count++;
		free(dirFile);
	}
	return fDentry;
}

Vfs::dentry * VfsFat::find_path(std::string absolute_path, size_t * start, size_t * end)
{
	std::string delimeter = "/";
	struct Vfs::dentry *mDentry = NULL;
	struct Vfs::dentry *fDentry = NULL;

	*start = 0;
	*end = absolute_path.find(delimeter);

	struct Vfs::super_block *sb = Vfs::find_super_block_by_name(absolute_path.substr(*start, *end - *start));
	if (sb == NULL) {
		return NULL;
	}
	mDentry = sb->s_root;
	*start = *end + delimeter.length();
	*end = absolute_path.find(delimeter, *start);


	while (*end != std::string::npos)
	{
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(*start, *end - *start), VFS_OBJECT_DIRECTORY);
		if (fDentry == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL;
		}

		mDentry = fDentry;

		*start = *end + delimeter.length();
		*end = absolute_path.find(delimeter, *start);
	}

	return mDentry;
}

int VfsFat::write_to_file(struct Vfs::file *file, char *buffer, int buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE) {
		// TODO mohu vymazat File, protoze f_dentry je NULL
		return -1;
	}

	struct dir_file *dirFile = new struct dir_file();
	strcpy_s(dirFile -> file_name, file->f_dentry->d_name.c_str());
	dirFile -> file_size = file->f_dentry->d_size;
	dirFile -> file_type = file->f_dentry->d_file_type;
	dirFile -> first_cluster = file->f_dentry->d_position;

	long writed_size = fat_write_file(dirFile, file->f_dentry->d_file_position, buffer, buffer_size, file->position);

	if (writed_size != 0) {
		file->f_dentry->d_size = dirFile -> file_size;
		file->f_dentry->d_blocks = (unsigned int) ceil((double)dirFile -> file_size / get_cluster_size());
	}

	delete dirFile;
	return writed_size;
}

int VfsFat::read_file(struct Vfs::file *file, char *buffer, int buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE) {
		// TODO mohu vymazat File, protoze f_dentry je NULL
		return -1;
	}

	struct dir_file dirFile;
	strcpy_s(dirFile.file_name, file->f_dentry->d_name.c_str());
	dirFile.file_size = file->f_dentry->d_size;
	dirFile.file_type = file->f_dentry->d_file_type;
	dirFile.first_cluster = file->f_dentry->d_position;

	return fat_read_file(dirFile, buffer, buffer_size, file->position);
}

int VfsFat::remove_file(struct Vfs::file **file)
{
	if (*file == NULL || (*file)->f_dentry == NULL || (*file)->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE || (*file)->f_dentry->d_count > 1) {
		// TODO mohu vymazat File, protoze f_dentry je NULL
		return -1;
	}

	int result = fat_delete_file_by_name((*file)->f_dentry->d_name.c_str(), (*file)->f_dentry->d_parent->d_position);

	if (result == 0) {
		sb_remove_file(file);
	}
	else {
		return -1;
	}

	return 0;
}

int VfsFat::close_file(struct Vfs::file **file)
{
	if (*file == NULL)
	{
		return -1;
	}

	Vfs::sb_remove_file(file);
	return 0;
}
