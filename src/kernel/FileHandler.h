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
	unsigned long position;
	

public:
	FileHandler(uint8_t flags, struct dentry *dentry, unsigned int count, unsigned long position) : Handler(flags), dentry(dentry), count(count), position(position) {};
	~FileHandler();
	bool read(char* buffer, size_t offset, size_t length, size_t& read);
	bool write(char* buffer, size_t offset, size_t length, size_t& written);

	struct dentry *get_dentry();
	unsigned long ftell();

	int fseek(long offset, uint8_t origin);
};