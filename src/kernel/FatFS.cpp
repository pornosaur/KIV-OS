#include "FatFS.h"

FatFS::FatFS(char *memory, size_t memory_size, const std::string &disk_id)
{

	f_data = new struct fat_data();
	f_data->memory = memory;
	f_data->memory_size = memory_size;

	int result = fat_init(f_data);

	if (result != 0 || FatFS::f_data->boot_record == NULL)
	{
		return; // TODO rict systemu ze nelze pracovat s FAT
	}

	struct super_block * sb = FS::init_super_block(
		FatFS::f_data->boot_record->cluster_size, 0,
		FatFS::f_data->boot_record->usable_cluster_count * FatFS::f_data->boot_record->cluster_size,
		NULL, 1, disk_id);

	struct dentry *root = FS::init_dentry(NULL, disk_id, 0, FatFS::f_data->start_of_root_dir, FS_OBJECT_DIRECTORY,
		FatFS::f_data->boot_record->cluster_size * FatFS::f_data->boot_record->dir_clusters, FatFS::f_data->boot_record->dir_clusters);
	root->d_count = 1;
	root->d_mounted = 1;

	sb->s_root = root;
}


FatFS::~FatFS()
{
	close_fat(f_data);
	delete f_data;
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

	return ERR_SUCCESS;
}

int FatFS::fs_create_dir(FileHandler **directory, const std::string &absolute_path)
{
	struct dentry *m_dentry = NULL;
	struct dentry *f_dentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*directory = NULL;

	m_dentry = FatFS::find_path(absolute_path, &start, &end);
	if (m_dentry == NULL) {
		return ERR_INVALID_PATH;
	}

	f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(start, end), FS_OBJECT_DIRECTORY);

	if (f_dentry == NULL) {
		unsigned long dir_position = 0;
		struct dir_file *d_file = NULL;
		int result = fat_create_dir(FatFS::f_data, &d_file, absolute_path.substr(start, end).c_str(), m_dentry->d_position, &dir_position);	
		if (d_file == NULL || result != 0) {
			FS::sb_remove_dentry(m_dentry);
			
			switch (result) {
			case 6:
				return ERR_DIRECTORY_IS_FULL;
			case 7:
				return ERR_DISK_IS_FULL;
			case 2:
			case 9:
				return ERR_INVALID_ARGUMENTS;
			case 12:
				return ERR_OUT_OF_MEMORY;
			default:
				return ERR_DISK_ERROR;
			}
		}

		f_dentry = FS::init_dentry(m_dentry, absolute_path.substr(start, end), d_file->first_cluster, dir_position, d_file->file_type,
			d_file->file_size, (unsigned long) ceil((double)d_file->file_size / FatFS::f_data->boot_record->cluster_size));
		free(d_file);
	}

	f_dentry->d_count++;
	*directory = new FileHandler(0, f_dentry, 0, 1);
	return ERR_SUCCESS;
}

int FatFS::fs_remove_emtpy_dir(FileHandler *file)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_file_type != FS::FS_OBJECT_DIRECTORY) {
		return ERR_INVALID_ARGUMENTS;
	}

	if (file->get_dentry()->d_parent == NULL || file->get_dentry()->d_mounted == 1) {
		return ERR_PERMISSION_DENIED;
	}

	if (file->get_dentry()->d_count > 1) {
		return ERR_FILE_OPEN_BY_OTHER;
	}

	int result = fat_delete_empty_dir(FatFS::f_data, file->get_dentry()->d_name.c_str(), file->get_dentry()->d_parent->d_position);

	switch (result) {
		case 0:
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

int FatFS::fs_read_dir(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_file_type != FS::FS_OBJECT_DIRECTORY || buffer == NULL || buffer_size == 0) {
		return ERR_INVALID_ARGUMENTS;
	}

	size_t i = 0;
	size_t buffer_position = 0;
	size_t record_size = sizeof(kiv_os::TDir_Entry);

	uint32_t read_size = 0;
	unsigned long *positions = new unsigned long[FatFS::f_data->max_dir_entries];
	struct dir_file *files = fat_read_dir(FatFS::f_data, file->get_dentry()->d_position, &read_size, positions);
	assert(read_size <= FatFS::f_data->max_dir_entries);

	size_t file_position = FatFS::f_data->start_of_root_dir + (file->get_dentry()->d_position * FatFS::f_data->boot_record->cluster_size);

	kiv_os::TDir_Entry dir_entry;
	for (i = 0; i < read_size; i++) {
		
		if (buffer_position + record_size > buffer_size) {
			break;
		}
		
		if (file_position + file->ftell() <= positions[i]) {
			strncpy_s(dir_entry.file_name, files[i].file_name, NAME_SIZE);
			dir_entry.file_attributes = 0;

			if (files[i].file_type == OBJECT_DIRECTORY) {
				dir_entry.file_attributes |= kiv_os::faDirectory;
			}

			memcpy(buffer + buffer_position, &dir_entry, record_size);
			buffer_position += record_size;

			file->fseek((long)(sizeof(struct dir_file) + positions[i] - file_position), kiv_os::fsBeginning, kiv_os::fsSet_Position);
		}
		
	}

	free(files);
	delete[] positions;

	*read_bytes = buffer_position;

	return ERR_SUCCESS;
}

int FatFS::fs_create_file(FileHandler **file, const std::string &absolute_path)
{
	struct dentry *m_dentry = NULL;
	struct dentry *f_dentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*file = NULL;

	m_dentry = FatFS::find_path(absolute_path, &start, &end);
	if (m_dentry == NULL) {
		return ERR_INVALID_PATH;
	}

	f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(start, end), FS_OBJECT_FILE);

	if (f_dentry != NULL) {
		if (f_dentry->d_count > 0) {
			return ERR_FILE_OPEN_BY_OTHER;
		}
		fat_delete_file_by_name(FatFS::f_data, f_dentry->d_name.c_str(), f_dentry->d_parent->d_position);
		FS::sb_remove_dentry(f_dentry);
		f_dentry = NULL;
	}

	unsigned long dir_position = 0;
	struct dir_file *d_file = NULL;
	int result  = fat_create_file(FatFS::f_data, &d_file, absolute_path.substr(start, end).c_str(), m_dentry->d_position, &dir_position);

	if (d_file == NULL || result != 0) {
		FS::sb_remove_dentry(m_dentry);
		
		switch (result) {
		case 6:
			return ERR_DIRECTORY_IS_FULL;
		case 7:
			return ERR_DISK_IS_FULL;
		case 2:
		case 9:
			return ERR_INVALID_ARGUMENTS;
		case 12:
			return ERR_OUT_OF_MEMORY;
		default:
			return ERR_DISK_ERROR;
		}
	}
	
	f_dentry = FS::init_dentry(m_dentry, absolute_path.substr(start, end), d_file->first_cluster, dir_position, d_file->file_type,
	d_file->file_size, (unsigned long) std::ceil((double)d_file->file_size / FatFS::f_data->boot_record->cluster_size));
	free(d_file);

	f_dentry->d_count++;
	*file = new FileHandler(0, f_dentry, 0, 1);
	return ERR_SUCCESS;
}

int FatFS::fs_open_object(FileHandler **object, const std::string &absolute_path, unsigned int type)
{
	struct dentry *m_dentry = NULL;
	struct dentry *f_dentry = NULL;
	size_t start = 0;
	size_t end = 0;
	*object = NULL;

	m_dentry = FatFS::find_path(absolute_path, &start, &end);
	if (m_dentry == NULL) {
		return ERR_INVALID_PATH;
	}

	f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(start, end), type);
	if (f_dentry == NULL) {
		FS::sb_remove_dentry(m_dentry);
		return ERR_FILE_NOT_FOUND;
	}
	
	f_dentry->d_count++;
	*object = new FileHandler(0, f_dentry, 0, 1);
	return ERR_SUCCESS;
}

struct dentry *FatFS::find_object_in_directory(struct dentry *m_dentry, const std::string& dentry_name, unsigned int type) {
	
	struct dentry *f_dentry = FS::sb_find_dentry_in_dentry(m_dentry, dentry_name, type);

	if (f_dentry == NULL) {
		unsigned long dir_position = 0;
		struct dir_file *d_file = fat_get_object_info_by_name(FatFS::f_data, dentry_name.c_str(), type, m_dentry->d_position, &dir_position);
		
		if (d_file == NULL) {
			return NULL;
		}

		f_dentry = FS::init_dentry(m_dentry, dentry_name, d_file->first_cluster, dir_position, d_file->file_type,
			d_file->file_size, (unsigned long)ceil((double)d_file->file_size / FatFS::f_data->boot_record->cluster_size));
		free(d_file);
	}
	return f_dentry;
}

dentry * FatFS::find_path(const std::string &absolute_path, size_t * start, size_t * end)
{
	std::string delimeter = "/";
	struct dentry *m_dentry = NULL;
	struct dentry *f_dentry = NULL;

	if (this->sb == NULL) {
		return NULL;
	}
	m_dentry = this->sb->s_root;

	*start = 0;
	*end = absolute_path.find(delimeter);


	while (*end != std::string::npos)
	{
		f_dentry = find_object_in_directory(m_dentry, absolute_path.substr(*start, *end - *start), FS_OBJECT_DIRECTORY);
		if (f_dentry == NULL) {
			FS::sb_remove_dentry(m_dentry);
			return NULL;
		}

		m_dentry = f_dentry;

		*start = *end + delimeter.length();
		*end = absolute_path.find(delimeter, *start);
	}

	return m_dentry;
}

int FatFS::fs_write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_file_type != FS::FS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}

	struct dir_file *d_file = create_dir_file(file->get_dentry());

	int result =  fat_write_file(FatFS::f_data, d_file, file->get_dentry()->d_dentry_position, buffer, (unsigned int) buffer_size, writed_bytes, (unsigned long)file->ftell());

	if (*writed_bytes != 0) {
		file->get_dentry()->d_size = d_file->file_size;
		file->get_dentry()->d_blocks = (unsigned long) ceil((double)d_file -> file_size / FatFS::f_data->boot_record->cluster_size);
	}
	
	delete d_file;

	switch (result) {
	case 0:
		return ERR_SUCCESS;
	case 7:
		return ERR_DISK_IS_FULL;
	case 12:
		return ERR_OUT_OF_MEMORY;
	default:
		return ERR_DISK_ERROR;
	}
}

int FatFS::fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_file_type != FS::FS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}

	struct dir_file *d_file = create_dir_file(file->get_dentry());

	int result = fat_read_file(FatFS::f_data, d_file, buffer, (unsigned int) buffer_size, read_bytes, (unsigned long)file->ftell());
	
	delete d_file;

	switch (result) {
	case 0:
		return ERR_SUCCESS;
	default:
		return ERR_DISK_ERROR;
	}
}

int FatFS::fs_remove_file(FileHandler *file)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_file_type != FS::FS_OBJECT_FILE) {
		return ERR_INVALID_ARGUMENTS;
	}
	if (file->get_dentry()->d_count > 1) {
		return ERR_FILE_OPEN_BY_OTHER;
	}

	int result = fat_delete_file_by_name(FatFS::f_data, file->get_dentry()->d_name.c_str(), file->get_dentry()->d_parent->d_position);


	switch (result) {
	case 0:
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

int FatFS::fs_set_file_size(FileHandler * file, size_t file_size)
{
	if (file == NULL || 
		file->get_dentry() == NULL || 
		file->get_dentry()->d_file_type != FS::FS_OBJECT_FILE ||
		file->get_dentry()->d_size < file_size) {
		return ERR_INVALID_ARGUMENTS;
	}

	struct dir_file *d_file = create_dir_file(file->get_dentry());

	int result = fat_set_file_size(FatFS::f_data, d_file, file_size, file->get_dentry()->d_dentry_position);

	uint32_t new_size = d_file->file_size;
	delete d_file;
	

	switch (result) {
		case 0:
			file->get_dentry()->d_size = new_size;
			file->get_dentry()->d_blocks = (unsigned long)ceil((double)new_size / FatFS::f_data->boot_record->cluster_size);
			return ERR_SUCCESS;

		case 8:
			return ERR_INVALID_ARGUMENTS;

		default:
			return ERR_DISK_ERROR;
	}
}


struct dir_file *FatFS::create_dir_file(struct dentry *dentry)
{	
	struct dir_file *d_file = new struct dir_file();
	strcpy_s(d_file->file_name, dentry->d_name.c_str());
	d_file->file_size = dentry->d_size;
	d_file->file_type = dentry->d_file_type;
	d_file->first_cluster = dentry->d_position;

	return d_file;
}