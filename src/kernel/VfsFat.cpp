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

	struct Vfs::dentry *root = Vfs::init_dentry(sb, NULL, "C:", 0, get_start_of_root_dir(), VFS_OBJECT_DIRECTORY,
		get_dir_size_in_bytes(), get_dir_clusters());
	root->d_count = 1;
	root->d_mounted = 1;

	sb->s_root = root;
}


VfsFat::~VfsFat()
{
	close_fat();
}

int VfsFat::create_dir(struct Vfs::file **directory, std::string absolute_path)
{
	struct Vfs::dentry *mDentry = NULL;
	struct Vfs::dentry *fDentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*directory = NULL;

	mDentry = VfsFat::find_path(absolute_path, &start, &end);
	if (mDentry == NULL) {
		return ERR_INVALID_PATH;
	}

	if (start == 0 && end == std::string::npos) {
		fDentry = mDentry;
	} 
	else {
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end), VFS_OBJECT_DIRECTORY);
	}

	if (fDentry == NULL) {
		unsigned long dir_position = 0;
		struct dir_file *dirFile = NULL;
		int result = fat_create_dir(&dirFile, absolute_path.substr(start, end).c_str(), mDentry->d_position, &dir_position);	
		if (dirFile == NULL || result != 0) {
			Vfs::sb_remove_dentry(mDentry);
			
			switch (result) {
			case 6:
				return ERR_DIRECTORY_IS_FULL;
			case 7:
				return ERR_DISK_IS_FULL;
			case 2:
			case 9:
				return ERR_INVALID_ARGUMENTS;
			default:
				return ERR_DISK_ERROR;
			}
		}

		fDentry = Vfs::init_dentry(sb, mDentry, absolute_path.substr(start, end), dirFile->first_cluster, dir_position, dirFile->file_type,
			dirFile->file_size, (unsigned long) ceil((double)dirFile->file_size / get_cluster_size()));
		free(dirFile);
	}

	fDentry->d_count++;
	*directory = Vfs::init_file(fDentry, 0, 0);
	return ERR_SUCCESS;
}

int VfsFat::remove_emtpy_dir(struct Vfs::file **file)
{
	if (*file == NULL || (*file)->f_dentry == NULL || (*file)->f_dentry->d_file_type != Vfs::VFS_OBJECT_DIRECTORY) {
		return ERR_INVALID_ARGUMENTS;
	}

	if ((*file)->f_dentry->d_parent == NULL || (*file)->f_dentry->d_mounted == 1) {
		return ERR_INVALID_PATH;
	}

	if ((*file)->f_dentry->d_count > 1) {
		return ERR_FILE_OPEN_BY_OTHER;
	}

	int result = fat_delete_empty_dir((*file)->f_dentry->d_name.c_str(), (*file)->f_dentry->d_parent->d_position);

	switch (result) {
		case 0:
			Vfs::sb_remove_file(file);
			return ERR_SUCCESS;
		case 3:
			return ERR_FILE_NOT_FOUND;
		case 5:
			return ERR_DIRECTORY_IS_NOT_EMPTY;
		case 2:
		case 9:
			return ERR_INVALID_ARGUMENTS;
		default:
			return ERR_DISK_ERROR;
	}
}

int VfsFat::read_dir(struct Vfs::file *file)
{
	return NULL;
	//TODO
}

int VfsFat::create_file(struct Vfs::file **file, std::string absolute_path)
{
	struct Vfs::dentry *mDentry = NULL;
	struct Vfs::dentry *fDentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*file = NULL;

	mDentry = VfsFat::find_path(absolute_path, &start, &end);
	if (mDentry == NULL) {
		return ERR_INVALID_PATH;
	}
	if (start == 0 && end == std::string::npos)
	{
		Vfs::sb_remove_dentry(mDentry);
		return ERR_INVALID_PATH;
	}

	fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end), VFS_OBJECT_FILE);

	if (fDentry != NULL) {
		if (fDentry->d_count > 0) {
			return ERR_FILE_OPEN_BY_OTHER;
		}
		fat_delete_file_by_name(fDentry->d_name.c_str(), fDentry->d_parent->d_position);
		Vfs::sb_remove_dentry(fDentry);
		fDentry = NULL;
	}

	unsigned long dir_position = 0;
	struct dir_file *dirFile = NULL;
	int result  = fat_create_file(&dirFile, absolute_path.substr(start, end).c_str(), mDentry->d_position, &dir_position);

	if (dirFile == NULL || result != 0) {
		Vfs::sb_remove_dentry(mDentry);
		
		switch (result) {
		case 6:
			return ERR_DIRECTORY_IS_FULL;
		case 7:
			return ERR_DISK_IS_FULL;
		case 2:
		case 9:
			return ERR_INVALID_ARGUMENTS;
		default:
			return ERR_DISK_ERROR;
		}
	}
	
	fDentry = Vfs::init_dentry(sb, mDentry, absolute_path.substr(start, end), dirFile->first_cluster, dir_position, dirFile->file_type,
	dirFile->file_size, (long) std::ceil((double)dirFile->file_size / get_cluster_size()));
	free(dirFile);

	fDentry->d_count++;
	*file =  Vfs::init_file(fDentry, 0, 0);
	return ERR_SUCCESS;
}

int VfsFat::open_object(struct Vfs::file **object, std::string absolute_path, unsigned int type)
{
	struct Vfs::dentry *mDentry = NULL;
	struct Vfs::dentry *fDentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*object = NULL;

	mDentry = VfsFat::find_path(absolute_path, &start, &end);
	if (mDentry == NULL) {
		return ERR_INVALID_PATH;
	}

	if (start == 0 && end == std::string::npos){
		if (type != Vfs::VFS_OBJECT_DIRECTORY)
		{
			Vfs::sb_remove_dentry(mDentry);
			return ERR_FILE_NOT_FOUND;
		}
		fDentry = mDentry;
	} 
	else
	{
		fDentry = find_object_in_directory(mDentry, absolute_path.substr(start, end), type);
		if (fDentry == NULL) {
			Vfs::sb_remove_dentry(mDentry);
			return ERR_FILE_NOT_FOUND;
		}
	}
	
	fDentry->d_count++;
	*object = Vfs::init_file(fDentry, 0, 0);
	return ERR_SUCCESS;
}

struct Vfs::dentry *VfsFat::find_object_in_directory(struct Vfs::dentry *mDentry, const std::string& dentry_name, unsigned int type) {
	
	struct Vfs::dentry *fDentry = Vfs::sb_find_dentry_in_dentry(mDentry, dentry_name, type);

	if (fDentry == NULL) {
		unsigned long dir_position = 0;
		struct dir_file *dirFile = fat_get_object_info_by_name(dentry_name.c_str(), type, mDentry->d_position, &dir_position);
		
		if (dirFile == NULL) {
			return NULL;
		}

		fDentry = Vfs::init_dentry(mDentry->d_sb, mDentry, dentry_name, dirFile->first_cluster, dir_position, dirFile->file_type,
			dirFile->file_size, (unsigned long)ceil((double)dirFile->file_size / get_cluster_size()));
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

int VfsFat::write_to_file(struct Vfs::file *file, size_t *writed_bytes, char *buffer, size_t buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}

	struct dir_file *dirFile = new struct dir_file();
	strcpy_s(dirFile -> file_name, file->f_dentry->d_name.c_str());
	dirFile -> file_size = file->f_dentry->d_size;
	dirFile -> file_type = file->f_dentry->d_file_type;
	dirFile -> first_cluster = file->f_dentry->d_position;

	*writed_bytes = fat_write_file(dirFile, file->f_dentry->d_dentry_position, buffer, (unsigned int) buffer_size, file->position);

	if (*writed_bytes != 0) {
		file->f_dentry->d_size = dirFile -> file_size;
		file->f_dentry->d_blocks = (unsigned long) ceil((double)dirFile -> file_size / get_cluster_size());
	}

	delete dirFile;
	return ERR_SUCCESS;
}

int VfsFat::read_file(struct Vfs::file *file, size_t *read_bytes, char *buffer, size_t buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}

	struct dir_file dirFile;
	strcpy_s(dirFile.file_name, file->f_dentry->d_name.c_str());
	dirFile.file_size = file->f_dentry->d_size;
	dirFile.file_type = file->f_dentry->d_file_type;
	dirFile.first_cluster = file->f_dentry->d_position;

	*read_bytes = fat_read_file(dirFile, buffer, (unsigned int) buffer_size, file->position);
	if (*read_bytes < 0) {
		return ERR_INVALID_ARGUMENTS;
	}

	return ERR_SUCCESS;
}

int VfsFat::remove_file(struct Vfs::file **file)
{
	if (*file == NULL || (*file)->f_dentry == NULL || (*file)->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}
	if ((*file)->f_dentry->d_count > 1) {
		return ERR_FILE_OPEN_BY_OTHER;
	}

	int result = fat_delete_file_by_name((*file)->f_dentry->d_name.c_str(), (*file)->f_dentry->d_parent->d_position);


	switch (result) {
	case 0:
		Vfs::sb_remove_file(file);
		return ERR_SUCCESS;
	case 3:
		return ERR_FILE_NOT_FOUND;
	case 2:
	case 9:
		return ERR_INVALID_ARGUMENTS;
	default:
		return ERR_DISK_ERROR;
	}
}

int VfsFat::close_file(struct Vfs::file **file)
{
	if (*file == NULL)
	{
		return ERR_INVALID_ARGUMENTS;
	}

	Vfs::sb_remove_file(file);
	return ERR_SUCCESS;
}
