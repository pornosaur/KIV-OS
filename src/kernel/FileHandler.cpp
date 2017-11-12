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

unsigned long FileHandler::get_position()
{
	return position;
}

void FileHandler::set_position(unsigned long pos)
{
	position = pos;
}
