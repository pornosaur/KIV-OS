#pragma once
class FileHandler
{

public:
	FileHandler() {};
	~FileHandler() {};
	/* Len = size buff */
	virtual size_t read(char* buffer, size_t offset, size_t length) = 0;
	virtual size_t write(char* buffer, size_t offset, size_t length) = 0;
};


