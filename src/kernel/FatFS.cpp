#include "FatFS.h"

FatFS::FatFS(char *memory, size_t memory_size)
{
	int result = fat_init(memory, memory_size);

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


FatFS::~FatFS()
{
	close_fat();
}

int FatFS::init_fat_disk(char *memory, size_t memory_size, uint16_t cluster_size) {

	uint8_t fat_copies = (uint8_t)2u;
	uint16_t b_record_size = sizeof(struct boot_record);
	uint16_t reserved_cluster_count = (b_record_size / cluster_size) + ((b_record_size % cluster_size) ? 1 : 0);
	uint16_t reserved_size = reserved_cluster_count * cluster_size;
	
	uint32_t usable_size = (uint32_t)(memory_size - reserved_size);
	uint32_t usable_cluster_count = usable_size / (cluster_size + (sizeof(uint32_t) * fat_copies)); // uint32_t size of record in fat table

	// create boot_record
	struct boot_record b_record;
	strncpy_s(b_record.volume_descriptor, "Virtual fat disk", 256);
	b_record.fat_type = (uint8_t)4096u;
	b_record.fat_copies = fat_copies;
	b_record.cluster_size = cluster_size;
	b_record.usable_cluster_count = usable_cluster_count;
	b_record.reserved_cluster_count = reserved_cluster_count;
	b_record.dir_clusters = 1;
	strncpy_s(b_record.signature, "OK", 9);

	// create FAT table
	int32_t *fat_table = new int32_t[usable_cluster_count];
	for (size_t i = 0; i < usable_cluster_count; i++) {
		fat_table[i] = FAT_UNUSED;
	}
	fat_table[0] = FAT_FILE_END;

	// write to memory
	size_t shift = 0;
	memcpy_s(memory, memory_size, &b_record, b_record_size);
	shift = reserved_size;
	memcpy_s(memory + shift, memory_size - shift, fat_table, sizeof(int32_t) * usable_cluster_count);
	shift += sizeof(int32_t) * usable_cluster_count;
	memcpy_s(memory + shift, memory_size - shift, fat_table, sizeof(int32_t) * usable_cluster_count);
	shift += sizeof(int32_t) * usable_cluster_count;
	for (size_t i = 0; i < cluster_size; i++) {
		memset(memory + shift + (i * sizeof(char)), '\0', sizeof(char));
	}

	delete[] fat_table;

	return 0;
}

int FatFS::create_dir(struct Vfs::file **directory, const std::string absolute_path)
{
	struct Vfs::dentry *m_dentry = NULL;
	struct Vfs::dentry *f_dentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*directory = NULL;

	m_dentry = FatFS::find_path(absolute_path, &start, &end);
	if (m_dentry == NULL) {
		return ERR_INVALID_PATH;
	}

	if (start == 0 && end == std::string::npos) {
		f_dentry = m_dentry;
	} 
	else {
		f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(start, end), VFS_OBJECT_DIRECTORY);
	}

	if (f_dentry == NULL) {
		unsigned long dir_position = 0;
		struct dir_file *d_file = NULL;
		int result = fat_create_dir(&d_file, absolute_path.substr(start, end).c_str(), m_dentry->d_position, &dir_position);	
		if (d_file == NULL || result != 0) {
			Vfs::sb_remove_dentry(m_dentry);
			
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

		f_dentry = Vfs::init_dentry(sb, m_dentry, absolute_path.substr(start, end), d_file->first_cluster, dir_position, d_file->file_type,
			d_file->file_size, (unsigned long) ceil((double)d_file->file_size / get_cluster_size()));
		free(d_file);
	}

	f_dentry->d_count++;
	*directory = Vfs::init_file(f_dentry, 0, 0);
	return ERR_SUCCESS;
}

int FatFS::remove_emtpy_dir(struct Vfs::file **file)
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

int FatFS::read_dir(struct Vfs::file *file)
{
	return NULL;
	//TODO
}

int FatFS::create_file(struct Vfs::file **file, const std::string absolute_path)
{
	struct Vfs::dentry *m_dentry = NULL;
	struct Vfs::dentry *f_dentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*file = NULL;

	m_dentry = FatFS::find_path(absolute_path, &start, &end);
	if (m_dentry == NULL) {
		return ERR_INVALID_PATH;
	}
	if (start == 0 && end == std::string::npos)
	{
		Vfs::sb_remove_dentry(m_dentry);
		return ERR_INVALID_PATH;
	}

	f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(start, end), VFS_OBJECT_FILE);

	if (f_dentry != NULL) {
		if (f_dentry->d_count > 0) {
			return ERR_FILE_OPEN_BY_OTHER;
		}
		fat_delete_file_by_name(f_dentry->d_name.c_str(), f_dentry->d_parent->d_position);
		Vfs::sb_remove_dentry(f_dentry);
		f_dentry = NULL;
	}

	unsigned long dir_position = 0;
	struct dir_file *d_file = NULL;
	int result  = fat_create_file(&d_file, absolute_path.substr(start, end).c_str(), m_dentry->d_position, &dir_position);

	if (d_file == NULL || result != 0) {
		Vfs::sb_remove_dentry(m_dentry);
		
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
	
	f_dentry = Vfs::init_dentry(sb, m_dentry, absolute_path.substr(start, end), d_file->first_cluster, dir_position, d_file->file_type,
	d_file->file_size, (unsigned long) std::ceil((double)d_file->file_size / get_cluster_size()));
	free(d_file);

	f_dentry->d_count++;
	*file =  Vfs::init_file(f_dentry, 0, 0);
	return ERR_SUCCESS;
}

int FatFS::open_object(struct Vfs::file **object, const std::string absolute_path, unsigned int type)
{
	struct Vfs::dentry *m_dentry = NULL;
	struct Vfs::dentry *f_dentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*object = NULL;

	m_dentry = FatFS::find_path(absolute_path, &start, &end);
	if (m_dentry == NULL) {
		return ERR_INVALID_PATH;
	}

	if (start == 0 && end == std::string::npos){
		if (type != Vfs::VFS_OBJECT_DIRECTORY)
		{
			Vfs::sb_remove_dentry(m_dentry);
			return ERR_FILE_NOT_FOUND;
		}
		f_dentry = m_dentry;
	} 
	else
	{
		f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(start, end), type);
		if (f_dentry == NULL) {
			Vfs::sb_remove_dentry(m_dentry);
			return ERR_FILE_NOT_FOUND;
		}
	}
	
	f_dentry->d_count++;
	*object = Vfs::init_file(f_dentry, 0, 0);
	return ERR_SUCCESS;
}

struct Vfs::dentry *FatFS::find_object_in_directory(struct Vfs::dentry *m_dentry, const std::string& dentry_name, unsigned int type) {
	
	struct Vfs::dentry *f_dentry = Vfs::sb_find_dentry_in_dentry(m_dentry, dentry_name, type);

	if (f_dentry == NULL) {
		unsigned long dir_position = 0;
		struct dir_file *d_file = fat_get_object_info_by_name(dentry_name.c_str(), type, m_dentry->d_position, &dir_position);
		
		if (d_file == NULL) {
			return NULL;
		}

		f_dentry = Vfs::init_dentry(m_dentry->d_sb, m_dentry, dentry_name, d_file->first_cluster, dir_position, d_file->file_type,
			d_file->file_size, (unsigned long)ceil((double)d_file->file_size / get_cluster_size()));
		free(d_file);
	}
	return f_dentry;
}

Vfs::dentry * FatFS::find_path(const std::string absolute_path, size_t * start, size_t * end)
{
	std::string delimeter = "/";
	struct Vfs::dentry *m_dentry = NULL;
	struct Vfs::dentry *f_dentry = NULL;

	*start = 0;
	*end = absolute_path.find(delimeter);

	struct Vfs::super_block *sb = Vfs::find_super_block_by_name(absolute_path.substr(*start, *end - *start));
	if (sb == NULL) {
		return NULL;
	}
	m_dentry = sb->s_root;
	*start = *end + delimeter.length();
	*end = absolute_path.find(delimeter, *start);


	while (*end != std::string::npos)
	{
		f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(*start, *end - *start), VFS_OBJECT_DIRECTORY);
		if (f_dentry == NULL) {
			Vfs::sb_remove_dentry(m_dentry);
			return NULL;
		}

		m_dentry = f_dentry;

		*start = *end + delimeter.length();
		*end = absolute_path.find(delimeter, *start);
	}

	return m_dentry;
}

int FatFS::write_to_file(struct Vfs::file *file, size_t *writed_bytes, char *buffer, size_t buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}

	struct dir_file *d_file = new struct dir_file();
	strcpy_s(d_file -> file_name, file->f_dentry->d_name.c_str());
	d_file -> file_size = file->f_dentry->d_size;
	d_file -> file_type = file->f_dentry->d_file_type;
	d_file -> first_cluster = file->f_dentry->d_position;

	*writed_bytes = fat_write_file(d_file, file->f_dentry->d_dentry_position, buffer, (unsigned int) buffer_size, file->position);

	if (*writed_bytes != 0) {
		file->f_dentry->d_size = d_file -> file_size;
		file->f_dentry->d_blocks = (unsigned long) ceil((double)d_file -> file_size / get_cluster_size());
	}

	delete d_file;
	return ERR_SUCCESS;
}

int FatFS::read_file(struct Vfs::file *file, size_t *read_bytes, char *buffer, size_t buffer_size)
{
	if (file == NULL || file->f_dentry == NULL || file->f_dentry->d_file_type != Vfs::VFS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}

	struct dir_file d_file;
	strcpy_s(d_file.file_name, file->f_dentry->d_name.c_str());
	d_file.file_size = file->f_dentry->d_size;
	d_file.file_type = file->f_dentry->d_file_type;
	d_file.first_cluster = file->f_dentry->d_position;

	*read_bytes = fat_read_file(d_file, buffer, (unsigned int) buffer_size, file->position);
	if (*read_bytes < 0) {
		return ERR_INVALID_ARGUMENTS;
	}

	return ERR_SUCCESS;
}

int FatFS::remove_file(struct Vfs::file **file)
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

int FatFS::close_file(struct Vfs::file **file)
{
	if (*file == NULL)
	{
		return ERR_INVALID_ARGUMENTS;
	}

	Vfs::sb_remove_file(file);
	return ERR_SUCCESS;
}