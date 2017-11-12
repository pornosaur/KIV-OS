#pragma once
#include "../api/api.h"

class Handler
{

private:
	uint8_t flags;

protected:
	size_t position;

public:
	static const uint8_t fmOpen_Write = 2;
	static const uint8_t fmOpen_Read = 3;

	Handler(uint8_t flags = 0, size_t position = 0) : flags(flags), position(position) {};
	~Handler() {};
	/* Len = size buff */
	virtual bool read(char* buffer, size_t length, size_t& read) = 0;
	virtual bool write(char* buffer, size_t length, size_t& written) = 0;
	
	virtual int fseek(long offset, uint8_t origin) = 0;
	
	size_t ftell() { return position; }
	uint8_t get_flags() { return flags; }
};


