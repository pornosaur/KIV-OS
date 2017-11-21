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
	
	/** Remove dentry from fs is is not NULL */
	~FileHandler();

	/**
	 * Read data from file/folder and save data to buffer.
	 *
	 * @param buffer which will be fill by read data
	 * @param length size of buffer
	 * @param read number of read bytes
	 * @return error codes which are defined in api.h
	 */
	uint16_t read(char* buffer, size_t length, size_t& read);

	/**
	 * Write data from buffer to file.
	 *
	 * @param buffer data which will be written to file
	 * @param length size of buffer
	 * @param written number of wrote bytes
	 * @return error codes which are defined in api.h
	 */
	uint16_t write(char* buffer, size_t length, size_t& written);

	/**
	 * Set file position. Based on origin is add offset to start of file, end of file or to current position.
	 * If set_size is set, final position will be set as new file size.
	 *
	 * @param offset sum with position based on origin
	 * @param origin constant which says if position will be set from 0 (fsBeggining), current position (fsCurrent), end of file (fsEnd).
	 * @param set_size if is set new position will be set as file size.
	 * @return error codes which are defined in api.h
	 */
	uint16_t fseek(long offset, uint8_t origin, uint8_t set_size);

	/** Only return associated dentry */
	struct dentry *get_dentry();
};