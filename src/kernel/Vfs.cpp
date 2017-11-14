#include "Vfs.h"


Vfs::Vfs()
{
}


Vfs::~Vfs()
{
	// TODO remove list of FS
	// Use shared pointer will be solution (FS is created somewhere and add to Vfs by register_FS)
}

uint16_t Vfs::create_dir(FileHandler ** directory, const std::string &absolute_path)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	if (path == "") {
		return kiv_os::erFile_Not_Found;
	}

	int ret_code = file_system->fs_create_dir(directory, path);

	return translate_return_codes(ret_code);
}

uint16_t Vfs::remove_emtpy_dir(FileHandler ** file)
{
	if (*file == NULL || (*file)->get_dentry() == NULL || (*file)->get_dentry()->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	FS * m_fs = (*file)->get_dentry()->d_fs;
	int ret_code = m_fs->fs_remove_emtpy_dir(file);
	if (ret_code == FS::ERR_SUCCESS) {
		(*file)->dec_count();
	}
	return translate_return_codes(ret_code);
}

uint16_t Vfs::read_dir(FileHandler * file)
{
	if (file == NULL || file->get_dentry() == NULL || file->get_dentry()->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	FS * m_fs = file->get_dentry()->d_fs;
	int ret_code = m_fs->fs_read_dir(file);
	
	return translate_return_codes(ret_code);
}

uint16_t Vfs::open_object(FileHandler ** object, const std::string &absolute_path, unsigned int type)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	int ret_code = file_system->fs_open_object(object, path, type);

	return translate_return_codes(ret_code);
}

uint16_t Vfs::create_file(FileHandler ** file, const std::string &absolute_path)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos)? "" :absolute_path.substr(end+1, std::string::npos);
	
	FS *file_system = Vfs::find_fs_by_name(disk);

	if (path == "") {
		return kiv_os::erFile_Not_Found;
	}

	int ret_code = file_system->fs_create_file(file, path);

	return translate_return_codes(ret_code);
}

uint16_t Vfs::write_to_file(Handler * file, size_t * writed_bytes, char * buffer, size_t buffer_size)
{
	if (file == NULL) {
		return kiv_os::erInvalid_Handle;
	}

	return file->write(buffer, buffer_size, *writed_bytes);
}

uint16_t Vfs::read_file(Handler * file, size_t * read_bytes, char * buffer, size_t buffer_size)
{
	if (file == NULL) {
		return kiv_os::erInvalid_Handle;
	}

	return file->read(buffer, buffer_size, *read_bytes);
}

uint16_t Vfs::remove_file(FileHandler ** file)
{
	if (*file == NULL || (*file)->get_dentry() == NULL || (*file)->get_dentry()->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	FS * m_fs = (*file)->get_dentry()->d_fs;

	int ret_code = m_fs->fs_remove_file(file);
	if (ret_code == FS::ERR_SUCCESS) {
		(*file)->dec_count();
	}
	return translate_return_codes(ret_code);
}

uint16_t Vfs::close_file(FileHandler ** file)
{
	if (*file == NULL || (*file)->get_dentry() == NULL || (*file)->get_dentry()->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	FS * m_fs = (*file)->get_dentry()->d_fs;

	int ret_code = m_fs->fs_close_file(file);
	if (ret_code == FS::ERR_SUCCESS) {
		(*file)->dec_count();
	}
	
	return translate_return_codes(ret_code);
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

uint16_t Vfs::translate_return_codes(int fs_ret_code)
{
	switch (fs_ret_code) {

	case FS::ERR_SUCCESS:
		return kiv_os::erSuccess;

	case FS::ERR_FILE_NOT_FOUND:
	case FS::ERR_INVALID_PATH:
		return kiv_os::erFile_Not_Found;

	case FS::ERR_DIRECTORY_IS_NOT_EMPTY:
		return kiv_os::erDir_Not_Empty;

	case FS::ERR_DIRECTORY_IS_FULL:
	case FS::ERR_DISK_IS_FULL:
		return kiv_os::erNo_Left_Space;

	case FS::ERR_FILE_OPEN_BY_OTHER:
		return kiv_os::erPermission_Denied;

	case FS::ERR_INVALID_ARGUMENTS:
	case FS::ERR_FS_EXISTS:
		return kiv_os::erInvalid_Argument;

	case FS::ERR_DISK_ERROR:
		return kiv_os::erIO;

	case FS::ERR_OUT_OF_MEMORY:
		return kiv_os::erOut_Of_Memory;
	}

	return kiv_os::erInvalid_Argument;
}
