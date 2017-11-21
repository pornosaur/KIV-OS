#include "Vfs.h"


Vfs::~Vfs()
{
	for (std::pair<std::string, FS*> pair : file_systems) delete pair.second;
}


uint16_t Vfs::create_dir(FileHandler ** directory, const std::string &absolute_path)
{
	size_t start = 0;
	size_t end = absolute_path.find(delimeter);

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	if (!file_system || path == "") {
		return kiv_os::erFile_Not_Found;
	}

	file_system->m_mutex.lock(); // lock FS
	int ret_code = file_system->fs_create_dir(directory, path);
	file_system->m_mutex.unlock(); // unlock FS

	return FS::translate_return_codes(ret_code);
}


uint16_t Vfs::remove_emtpy_dir(const std::string &absolute_path)
{
	FileHandler *file = NULL;
	uint16_t ret_code1 = Vfs::open_object(&file, absolute_path, FS::FS_OBJECT_DIRECTORY);

	if (ret_code1 != kiv_os::erSuccess) {
		return ret_code1;
	}

	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	FS * m_fs = file->get_dentry()->d_fs;

	m_fs->m_mutex.lock(); // lock FS
	int ret_code2 = m_fs->fs_remove_emtpy_dir(file);
	m_fs->m_mutex.unlock(); // unlock FS

	assert(file->get_count() == 1);
	delete file;

	return FS::translate_return_codes(ret_code2);
}


uint16_t Vfs::open_object(FileHandler ** object, const std::string &absolute_path, unsigned int type)
{
	size_t start = 0;
	size_t end = absolute_path.find(delimeter);

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	if (!file_system) {
		return kiv_os::erFile_Not_Found;
	}
	file_system->m_mutex.lock(); // lock FS
	int ret_code = file_system->fs_open_object(object, path, type);
	file_system->m_mutex.unlock(); // unlock FS

	return FS::translate_return_codes(ret_code);
}


uint16_t Vfs::create_file(FileHandler ** file, const std::string &absolute_path)
{
	size_t start = 0;
	size_t end = absolute_path.find(delimeter);

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos)? "" :absolute_path.substr(end+1, std::string::npos);
	
	FS *file_system = Vfs::find_fs_by_name(disk);

	if (!file_system || path == "") {
		return kiv_os::erFile_Not_Found;
	}

	file_system->m_mutex.lock(); // lock FS
	int ret_code = file_system->fs_create_file(file, path);
	file_system->m_mutex.unlock(); // unlock FS

	return FS::translate_return_codes(ret_code);
}


uint16_t Vfs::remove_file(const std::string &absolute_path)
{

	FileHandler *file = NULL;
	uint16_t ret_code1 = Vfs::open_object(&file, absolute_path, FS::FS_OBJECT_FILE);

	if (ret_code1 != kiv_os::erSuccess) {
		return ret_code1;
	}

	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	FS * m_fs = file->get_dentry()->d_fs;

	m_fs->m_mutex.lock(); // lock FS
	int ret_code2 = m_fs->fs_remove_file(file);
	m_fs->m_mutex.unlock(); // unlock FS

	assert(file->get_count() == 1);
	delete file;

	return FS::translate_return_codes(ret_code2);
}


uint16_t Vfs::register_fs(const std::string &name, FS * fs)
{
	if (fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	if (Vfs::find_fs_by_name(name) != NULL) {
		return kiv_os::erInvalid_Argument;
	}

	file_systems.insert(std::pair<std::string, FS*>(name, fs));
	return kiv_os::erSuccess;
}


FS *Vfs::find_fs_by_name(const std::string &name)
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
