#pragma once
#include "../api/api.h"

class Handler
{

protected:
	size_t position;	/* Represents position in file */
	size_t count;		/* Represents how much processes have this handler */
	uint8_t flags;		/* Represents how handler is opened - read, write, always_open */

public:
	static const uint8_t fmOpen_Write = 2;
	static const uint8_t fmOpen_Read = 3;

	Handler(uint8_t flags = 0, size_t position = 0, size_t count = 0) : flags(flags), position(position), count(count) {};
	~Handler() {};
	
	virtual uint16_t read(char* buffer, size_t length, size_t& read) = 0;		/* length = size buff */
	virtual uint16_t write(char* buffer, size_t length, size_t& written) = 0;	/* length = size buffer*/
	
	virtual uint16_t fseek(long offset, uint8_t origin) = 0;
	
	size_t ftell() { return position; }
	uint8_t get_flags() { return flags; }
	size_t get_count() { return count; }
	void inc_count() { count++; }
	void dec_count()
	{ 
		if (count > 0) {
			count--;
		}
	}
};


