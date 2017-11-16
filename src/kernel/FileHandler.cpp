#include "FileHandler.h"

FileHandler::~FileHandler()
{
	if (dentry != NULL)
	{
		dentry->d_count--;
		dentry->d_fs->sb_remove_dentry(dentry); // nesmaze se kdyz na nej nekdo odkazuje
		dentry = NULL;
	}
}

uint16_t FileHandler::read(char * buffer, size_t length, size_t & read)
{
	if (dentry == NULL || dentry->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	FS * m_fs = dentry->d_fs;
	int ret_code = 0;
	
	if (dentry->d_file_type == FS::FS_OBJECT_DIRECTORY) {
		ret_code = m_fs->fs_read_dir(this, &read, buffer, length); // directory
	} else {
		ret_code = m_fs->fs_read_file(this, &read, buffer, length); // not directory
	}
	
	return FS::translate_return_codes(ret_code);
	
}

uint16_t FileHandler::write(char * buffer, size_t length, size_t & written)
{
	if (dentry == NULL || dentry->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}
	
	FS * m_fs = dentry->d_fs;
	int ret_code = m_fs->fs_write_to_file(this, &written, buffer, length);

	return FS::translate_return_codes(ret_code);
}

uint16_t FileHandler::fseek(long offset, uint8_t origin, uint8_t set_size)
{
	long new_position = (long)position;
	if (dentry == NULL || dentry->d_fs == NULL) {
		return kiv_os::erInvalid_Argument;
	}

	switch (origin) {
		case kiv_os::fsBeginning:
			new_position = offset;
			break;
		case kiv_os::fsCurrent:
			new_position += offset;
			break;
		case kiv_os::fsEnd:
			new_position = dentry->d_size + offset;
			break;
		default:
			return kiv_os::erInvalid_Argument;
	}

	if (new_position < 0 || (unsigned long)new_position > dentry->d_size) {
		return kiv_os::erInvalid_Argument;
	}

	if (set_size) {
		FS * m_fs = dentry->d_fs;
		int ret_code = m_fs->fs_set_file_size(this, (size_t)new_position);

		return FS::translate_return_codes(ret_code);
	}

	position = (size_t)new_position;
	return kiv_os::erSuccess;
}

dentry * FileHandler::get_dentry()
{
	return dentry;
}
