#pragma once

#include "Handler.h"
#include "VfsStructures.h"
#include "FS.h"
#include "../api/api.h"


class FileHandler : public Handler
{

private:
	struct dentry *dentry = NULL;
	

public:
	FileHandler(uint8_t flags = 0, struct dentry *dentry = NULL, size_t position = 0, size_t count = 0) : Handler(flags, position, count), dentry(dentry) {};
	~FileHandler();
	uint16_t read(char* buffer, size_t length, size_t& read);
	uint16_t write(char* buffer, size_t length, size_t& written);
	uint16_t fseek(long offset, uint8_t origin);

	struct dentry *get_dentry();
};