#include "VfsFat.h"

VfsFat::VfsFat()
{
	fat_init("output.fat");


	Vfs::init_super_block(
		boot_record->cluster_size, 0,
		boot_record->usable_cluster_count * boot_record->cluster_size,
		NULL, 1, "FAT_DISK");

	struct Vfs::dentry *root = Vfs::init_dentry(&(Vfs::sb), NULL, "C:", 1, 1, 0, start_of_root_dir, VFS_OBJECT_DIRECTORY, 
		boot_record->cluster_size * boot_record->dir_clusters, boot_record->dir_clusters, 0, NULL, NULL);

	Vfs::root_file.f_count = 0;
	Vfs::root_file.f_dentry = root;
	Vfs::root_file.position = 0;

	Vfs::sb.s_root = root;
}


VfsFat::~VfsFat()
{
	close_fat();
}

struct Vfs::file *VfsFat::create_dir(std::string absolute_path)
{
	std::string delimeter = "/";
	struct Vfs::dentry *mDentry = Vfs::sb.s_root;
	struct Vfs::dentry *fDentry = NULL;

	unsigned long start = 0;
	unsigned long end = absolute_path.find(delimeter);

	while (end != std::string::npos)
	{
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end - start));
		if (fDentry == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL;
		}
		else if (fDentry->d_file_type == VFS_OBJECT_FILE)
		{
			Vfs::sb_remove_dentry(fDentry);
			return NULL;
		}

		mDentry = fDentry;

		start = end + delimeter.length();
		end = absolute_path.find(delimeter, start);
	}

	fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end));


	if (fDentry == NULL) {
		long dir_position = 0;
		struct dir_file *dirFile = fat_create_dir(absolute_path.substr(start, end).c_str(), mDentry->d_position, &dir_position);	
		if (dirFile == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL; // cant create file on physic medium
		}

		fDentry = Vfs::init_dentry(&(Vfs::sb), mDentry, absolute_path.substr(start, end), 0, 0, dirFile->first_cluster, dir_position, dirFile->file_type,
			dirFile->file_size, ceil(dirFile->file_size / boot_record->cluster_size), 0, NULL, NULL);
		fDentry->d_next_subdir = mDentry->d_subdirectories;
		mDentry->d_subdirectories = fDentry;
		mDentry->d_count++;
		free(dirFile);
	}
	else if (fDentry->d_file_type == VFS_OBJECT_FILE)
	{
		Vfs::sb_remove_dentry(fDentry);
		return NULL;
	}

	fDentry->d_count++;
	return Vfs::init_file(fDentry, 0, 0);
}

int VfsFat::remove_emtpy_dir(struct Vfs::file *file)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_parent == NULL || file->f_dentry->d_file_type != 0) {
		// TODO mohu vymazat File, protoze f_dentry je NULL
		return -1;
	}

	int result = fat_delete_empty_dir(file->f_dentry->d_name.c_str(), file->f_dentry->d_parent->d_position);

	if (result == 0) {
		Vfs::sb_remove_file(file);
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
	std::string delimeter = "/";
	struct Vfs::dentry *mDentry = Vfs::sb.s_root;
	struct Vfs::dentry *fDentry = NULL;

	unsigned long start = 0;
	unsigned long end = absolute_path.find(delimeter);

	while (end != std::string::npos)
	{
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end - start));
		if (fDentry == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL;
		}
		else if (fDentry->d_file_type == VFS_OBJECT_FILE)
		{
			Vfs::sb_remove_dentry(fDentry);
			return NULL;
		}

		mDentry = fDentry;

		start = end + delimeter.length();
		end = absolute_path.find(delimeter, start);
	}

	fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end));


	if (fDentry == NULL) {
		long dir_position = 0;
		struct dir_file *dirFile = fat_create_file(absolute_path.substr(start, end).c_str(), mDentry->d_position, &dir_position);
		if (dirFile == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL; // cant create file on physic medium
		}

		fDentry = Vfs::init_dentry(&(Vfs::sb), mDentry, absolute_path.substr(start, end), 0, 0, dirFile->first_cluster, dir_position, dirFile->file_type,
			dirFile->file_size, ceil(dirFile->file_size / boot_record->cluster_size), 0, NULL, NULL);
		fDentry->d_next_subdir = mDentry->d_subdirectories;
		mDentry->d_subdirectories = fDentry;
		mDentry->d_count++;
		free(dirFile);
	} 
	else if (fDentry->d_file_type == VFS_OBJECT_DIRECTORY)
	{
		Vfs::sb_remove_dentry(fDentry);
		return NULL;
	}

	fDentry->d_count++;
	return Vfs::init_file(fDentry, 0, 0);
}

struct Vfs::file *VfsFat::open_file(std::string absolute_path)
{
	std::string delimeter = "/";
	struct Vfs::dentry *mDentry = Vfs::sb.s_root;
	struct Vfs::dentry *fDentry = NULL;
	
	unsigned long start = 0;
	unsigned long end = absolute_path.find(delimeter);

	while (end != std::string::npos)
	{
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end - start));
		if (fDentry == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return NULL;
		} 
		else if (fDentry->d_file_type == VFS_OBJECT_FILE)
		{
			Vfs::sb_remove_dentry(fDentry);
			return NULL;
		}

		mDentry = fDentry;
		
		start = end + delimeter.length();
		end = absolute_path.find(delimeter, start);
	}

	fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end));
	if (fDentry == NULL) {
		Vfs::sb_remove_dentry(mDentry);
		return NULL;
	}
	else if (fDentry->d_file_type == VFS_OBJECT_DIRECTORY)
	{
		Vfs::sb_remove_dentry(fDentry);
		return NULL;
	}

	fDentry->d_count++;
	return Vfs::init_file(fDentry, 0, 0);
}

struct Vfs::dentry *VfsFat::find_object_in_directory(struct Vfs::dentry *mDentry, const std::string& dentry_name) {
	
	struct Vfs::dentry *fDentry = Vfs::sb_find_dentry_in_dentry(mDentry, dentry_name);

	if (fDentry == NULL) {
		long dir_position = 0;
		struct dir_file *dirFile = fat_get_object_info_by_name(dentry_name.c_str(), mDentry->d_position, &dir_position);
		
		if (dirFile == NULL) {
			return NULL;
		}

		fDentry = Vfs::init_dentry(&(Vfs::sb), mDentry, dentry_name, 0, 0, dirFile->first_cluster, dir_position, dirFile->file_type,
			dirFile->file_size, ceil(dirFile->file_size / boot_record->cluster_size), 0, NULL, NULL);
		fDentry->d_next_subdir = mDentry->d_subdirectories;
		mDentry->d_subdirectories = fDentry;
		mDentry->d_count++;
		free(dirFile);
	}
	return fDentry;
}

int VfsFat::write_to_file(struct Vfs::file *file, char *buffer, int buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != 1) {
		// TODO mohu vymazat File, protoze f_dentry je NULL
		return -1;
	}

	struct dir_file dirFile;
	strcpy_s(dirFile.file_name, file->f_dentry->d_name.c_str());
	dirFile.file_size = file->f_dentry->d_size;
	dirFile.file_type = file->f_dentry->d_file_type;
	dirFile.first_cluster = file->f_dentry->d_position;

	long writed_size = fat_write_file(&dirFile, file->f_dentry->d_file_position, buffer, buffer_size, file->position);

	if (writed_size != 0) {
		file->f_dentry->d_size = dirFile.file_size;
		file->f_dentry->d_blocks = ceil(dirFile.file_size / boot_record->cluster_size);
	}

	return writed_size;
}

int VfsFat::read_file(struct Vfs::file *file, char *buffer, int buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != 1) {
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

int VfsFat::remove_file(struct Vfs::file *file)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != 1) {
		// TODO mohu vymazat File, protoze f_dentry je NULL
		return -1;
	}

	int result = fat_delete_file_by_name(file->f_dentry->d_name.c_str(), file->f_dentry->d_parent->d_position);

	if (result == 0) {
		sb_remove_file(file);
	}

	return 0;
}

int VfsFat::close_file(struct Vfs::file *file)
{
	if (file == NULL || file->f_dentry == NULL) {
		return -1;
	}

	file->f_dentry->d_count--;
	Vfs::sb_remove_dentry(file->f_dentry); // !! cant close root dir

	file->f_dentry = NULL;
	return 0;
}
