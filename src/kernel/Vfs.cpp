#include "Vfs.h"


Vfs::Vfs()
{
}


Vfs::~Vfs()
{
	// TODO remove list of FS
	// Use shared pointer will be solution (FS is created somewhere and add to Vfs by register_FS)
}

int Vfs::create_dir(FileHandler ** directory, const std::string absolute_path)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	if (path == "") {
		return FS::ERR_INVALID_PATH;
	}

	return file_system->fs_create_dir(directory, path);
}

int Vfs::remove_emtpy_dir(FileHandler ** file)
{
	if (*file == NULL || (*file)->get_dentry() == NULL || (*file)->get_dentry()->d_fs == NULL) {
		return FS::ERR_INVALID_ARGUMENTS;
	}

	FS * m_fs = (*file)->get_dentry()->d_fs;
	int result = m_fs->fs_remove_emtpy_dir(file);
	if (result == FS::ERR_SUCCESS) {
		Vfs::sb_remove_file(file);
	}
	return result;
}

int Vfs::read_dir(FileHandler * file)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_fs == NULL) {
		return FS::ERR_INVALID_ARGUMENTS;
	}

	FS * m_fs = file->get_dentry()->d_fs;
	return m_fs->fs_read_dir(file);
}

int Vfs::open_object(FileHandler ** object, const std::string absolute_path, unsigned int type)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	return file_system->fs_open_object(object, path, type);
}

int Vfs::create_file(FileHandler ** file, const std::string absolute_path)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos)? "" :absolute_path.substr(end+1, std::string::npos);
	
	FS *file_system = Vfs::find_fs_by_name(disk);

	if (path == "") {
		return FS::ERR_INVALID_PATH;
	}

	return file_system->fs_create_file(file, path);
}

int Vfs::write_to_file(FileHandler * file, size_t * writed_bytes, char * buffer, size_t buffer_size)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_fs == NULL) {
		return FS::ERR_INVALID_ARGUMENTS;
	}

	FS * m_fs = file->get_dentry()->d_fs;
	return m_fs->fs_write_to_file(file, writed_bytes, buffer, buffer_size);
}

int Vfs::read_file(FileHandler * file, size_t * read_bytes, char * buffer, size_t buffer_size)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_fs == NULL) {
		return FS::ERR_INVALID_ARGUMENTS;
	}

	FS * m_fs = file->get_dentry()->d_fs;
	return m_fs->fs_read_file(file, read_bytes, buffer, buffer_size);
}

int Vfs::remove_file(FileHandler ** file)
{
	if (*file == NULL || (*file)->get_dentry() == NULL || (*file)->get_dentry()->d_fs == NULL) {
		return FS::ERR_INVALID_ARGUMENTS;
	}

	FS * m_fs = (*file)->get_dentry()->d_fs;

	int result = m_fs->fs_remove_file(file);
	if (result == FS::ERR_SUCCESS) {
		Vfs::sb_remove_file(file);
	}
	return result;
}

int Vfs::close_file(FileHandler ** file)
{
	if (*file == NULL || (*file)->get_dentry() == NULL || (*file)->get_dentry()->d_fs == NULL) {
		return FS::ERR_INVALID_ARGUMENTS;
	}

	FS * m_fs = (*file)->get_dentry()->d_fs;

	int result = m_fs->fs_close_file(file);
	if (result == FS::ERR_SUCCESS) {
		Vfs::sb_remove_file(file);
	}
	return result;

}


FS *Vfs::find_fs_by_name(const std::string name)
{	
	auto it = Vfs::file_systems.find(name);

	if (it == Vfs::file_systems.end()) {
		return NULL;
	}
	else
	{
		return it->second;
	}

	return NULL;
}

int Vfs::sb_remove_file(FileHandler **file) { // TODO this could be in FileHandler destructor
	if ((*file) == NULL) {
		return -1;
	}

	if ((*file)->get_dentry() != NULL)
	{
		(*file)->get_dentry()->d_count--;
		(*file)->get_dentry()->d_fs->sb_remove_dentry((*file)->get_dentry()); // nesmaze se kdyz na nej nekdo odkazuje
		//(*file)->set_dentry(NULL);
	}

	delete *file;
	(*file) = NULL;

	return 0;
}

int Vfs::set_file_position(FileHandler * file, long offset, uint8_t origin)
{
	int result = -1;
	
	if (file != NULL && file->get_dentry() != NULL) {
		result = file->fseek(offset, origin);
	}

	return result;
}

unsigned long Vfs::get_file_position(FileHandler * file)
{
	if (file == NULL) {
		return 0;
	}
	else {
		return file->ftell();
	}
}

int Vfs::register_fs(const std::string name, FS * fs)
{
	if (fs == NULL) {
		return FS::ERR_INVALID_ARGUMENTS;
	}

	if (Vfs::find_fs_by_name(name) != NULL) {
		return FS::ERR_FS_EXISTS;
	}

	file_systems.insert(std::pair<std::string, FS*>(name, fs));
	return FS::ERR_SUCCESS;
}
