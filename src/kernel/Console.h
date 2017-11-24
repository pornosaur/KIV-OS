#pragma once

#include "..\api\api.h"
#include <Windows.h>
#include "Handler.h"

class Console : public Handler
{
public:
	Console(kiv_os::THandle);
	~Console();
	/**
	* Read data from console and save data to buffer.
	*
	* @param buffer which will be fill by read data
	* @param length size of buffer
	* @param read number of read bytes
	* @return error codes which are defined in api.h
	*/
	uint16_t read(char* buffer, size_t length, size_t& read);
	/**
	* Write data from buffer to console.
	*
	* @param buffer data which will be written to console
	* @param length size of buffer
	* @param written number of wrote bytes
	* @return error codes which are defined in api.h
	*/
	uint16_t write(char* buffer, size_t length, size_t& written);
	/**
	* Not allow for console 
	* @return error code erPermission_Denied which is defined in api.h
	*/
	uint16_t fseek(long offset, uint8_t origin, uint8_t set_size);

private:
	HANDLE mStdIn;					/* handle for stdin */
	HANDLE mStdOut;					/* handle for stdout */
	HANDLE mStdError;				/* handle for stderr */
	bool mStdInOpen = false;		/* flag for input console */
	bool mStdOutOpen = false;		/* flag for output console */
	bool mStdErrorOpen = false;		/* flag for error console */
};

