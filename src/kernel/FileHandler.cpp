#include "FileHandler.h"

FileHandler::~FileHandler()
{
}

bool FileHandler::read(char * buffer, size_t offset, size_t length, size_t & read)
{
	return false;
}

bool FileHandler::write(char * buffer, size_t offset, size_t length, size_t & written)
{
	return false;
}

dentry * FileHandler::get_dentry()
{
	return dentry;
}

unsigned long FileHandler::ftell()
{
	return position;
}

int FileHandler::fseek(long offset, uint8_t origin)
{
	long new_position = 0;
	if (dentry == NULL) {
		return -1;
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
			return -1;
	}

	if (new_position < 0 || (unsigned long)new_position > dentry->d_size) {
		return -1;
	}

	position = (unsigned long)new_position;
	return 0;
}
