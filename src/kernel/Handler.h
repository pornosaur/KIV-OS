#pragma once

#undef stdin
#undef stdout
#undef stderr

#include "../api/api.h"

class Handler
{

protected:
	size_t position;	/* Represents position in file */
	size_t count;		/* Represents how much processes have this handler */
	uint8_t flags;		/* Represents how handler is opened - read, write, always_open */

public:
	/* Constant which represents handler for write */
	static const uint8_t fmOpen_Write = 2;
	/* Constant which represents handler for read */
	static const uint8_t fmOpen_Read = 3;

	Handler(uint8_t flags = 0, size_t position = 0, size_t count = 0) : flags(flags), position(position), count(count) {};
	virtual ~Handler() {};
	
	/**
	 * Abstract method for read each handler which inherit this class.
	 *
	 * @param buffer input buffer
	 * @param length size of input buffer
	 * @param read read byte to input buffer
	 * @return error code
	 */
	virtual uint16_t read(char* buffer, size_t length, size_t& read) = 0;		/* length = size buff */

	/**
	 * Abstract method for write each handler which inherit this class.
	 *
	 * @param buffer output buffer
	 * @param length size of output buffer
	 * @param written written byte to output buffer
	 * @param error code
	 */
	virtual uint16_t write(char* buffer, size_t length, size_t& written) = 0;	/* length = size buffer*/
	
	/**
	 * Abstract method for set position in factual handler which inherit this class.
	 *
	 * @param offset number of bytes to offset from origin
	 * @param origin position which reference for offset
	 * @param set_size TODO comment this
	 * @return error code
	 */
	virtual uint16_t fseek(long offset, uint8_t origin, uint8_t set_size) = 0; /* origin = api fs const; set_size = api fsSet const */
	
	size_t ftell() { return position; }
	uint8_t get_flags() { return flags; }
	size_t get_count() { return count; }
	void inc_count() { count++; }
	void dec_count() { if (count > 0) count--; }
	
	/** decrement counter and return true if counter is 0 */
	bool close_handler() {
		dec_count();
		return get_count() <= 0;
	}
};


