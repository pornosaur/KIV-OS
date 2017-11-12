#pragma once

#include "Handler.h"
#include "VfsStructures.h"
#include "FS.h"
#include "../api/api.h"


class FileHandler : public Handler
{

private:
	struct dentry *dentry = NULL;
	unsigned int count;
	

public:
	FileHandler(uint8_t flags = 0, struct dentry *dentry = NULL, unsigned int count = 0, size_t position = 0) : Handler(flags, position), dentry(dentry), count(count) {};
	~FileHandler();
	bool read(char* buffer, size_t length, size_t& read);
	bool write(char* buffer, size_t length, size_t& written);
	int fseek(long offset, uint8_t origin);

	struct dentry *get_dentry();
};