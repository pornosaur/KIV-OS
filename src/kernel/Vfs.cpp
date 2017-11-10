#include "Vfs.h"


Vfs::Vfs()
{
}


Vfs::~Vfs()
{
	delete fs;
	// TODO remove list of FS
}

int Vfs::create_dir(FS::file ** directory, const std::string absolute_path)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	if (path == "") {
		return FS::ERR_INVALID_PATH;
	}

	return fs->fs_create_dir(directory, path);
}

int Vfs::remove_emtpy_dir(FS::file ** file)
{
	int result = fs->fs_remove_emtpy_dir(file);
	if (result == FS::ERR_SUCCESS) {
		Vfs::sb_remove_file(file);
	}
	return result;
}

int Vfs::read_dir(FS::file * file)
{
	return fs->fs_read_dir(file);
}

int Vfs::open_object(FS::file ** object, const std::string absolute_path, unsigned int type)
{
	size_t start = 0;
	size_t end = absolute_path.find("/");

	const std::string disk = absolute_path.substr(start, end);
	const std::string path = (end == std::string::npos) ? "" : absolute_path.substr(end + 1, std::string::npos);

	FS *file_system = Vfs::find_fs_by_name(disk);

	return file_system->fs_open_object(object, path, type);
}

int Vfs::create_file(FS::file ** file, const std::string absolute_path)
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

int Vfs::write_to_file(FS::file * file, size_t * writed_bytes, char * buffer, size_t buffer_size)
{
	return fs->fs_write_to_file(file, writed_bytes, buffer, buffer_size);
}

int Vfs::read_file(FS::file * file, size_t * read_bytes, char * buffer, size_t buffer_size)
{
	return fs->fs_read_file(file, read_bytes, buffer, buffer_size);
}

int Vfs::remove_file(FS::file ** file)
{
	int result = fs->fs_remove_file(file);
	if (result == FS::ERR_SUCCESS) {
		Vfs::sb_remove_file(file);
	}
	return result;
}

int Vfs::close_file(FS::file ** file)
{
	int result = fs->fs_close_file(file);
	if (result == FS::ERR_SUCCESS) {
		Vfs::sb_remove_file(file);
	}
	return result;

}


FS *Vfs::find_fs_by_name(const std::string name)
{
	
	// TODO implement
	return fs;

	return NULL;
}

int Vfs::sb_remove_file(struct FS::file **file) {
	if ((*file) == NULL) {
		return -1;
	}

	if ((*file)->f_dentry != NULL)
	{
		(*file)->f_dentry->d_count--;
		(*file)->f_dentry->d_fs->sb_remove_dentry((*file)->f_dentry); // nesmaze se kdyz na nej nekdo odkazuje
		(*file)->f_dentry = NULL;
	}

	delete *file;
	(*file) = NULL;

	return 0;
}

void Vfs::set_file_position(struct FS::file * file, unsigned long position)
{
	if (file != NULL || file->f_dentry == NULL) {
		if (file->f_dentry->d_size < position) {
			file->position = file->f_dentry->d_size;
		}
		else if (position < 0) {
			file->position = 0;
		}
		else {
			file->position = position;
		}
	}
}

unsigned long Vfs::get_file_position(FS::file * file)
{
	if (file == NULL) {
		return -1;
	}
	else {
		return file->position;
	}
}

int Vfs::register_fs(FS * fs)
{
	this->fs = fs;
	return FS::ERR_SUCCESS;
}
