#pragma once

#include "..\api\api.h"
#include <Windows.h>
#include "FileHandler.h"

class Console : public FileHandler
{
public:
	Console(kiv_os::THandle);
	~Console();

	bool read(char* buffer, size_t offset, size_t length, size_t& read);
	bool write(char* buffer, size_t offset, size_t length, size_t& written);
private:
	HANDLE mStdIn;
	HANDLE mStdOut;
	HANDLE mStdError;
	bool mStdInOpen; 
	bool mStdOutOpen;
	bool mStdErrorOpen;
};
