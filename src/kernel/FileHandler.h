#pragma once
#include "../api/api.h"

class FileHandler
{

private:
	uint8_t flags;

public:
	static const uint8_t fmOpen_Write = 2;
	static const uint8_t fmOpen_Read = 3;

	FileHandler(uint8_t flags = 0) : flags(flags) {};
	~FileHandler() {};
	/* Len = size buff */
	virtual bool read(char* buffer, size_t offset, size_t length, size_t& read) = 0;
	virtual bool write(char* buffer, size_t offset, size_t length, size_t& written) = 0;

	uint8_t get_flags() { return flags; }
};


