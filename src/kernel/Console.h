#pragma once

#include "..\api\api.h"
#include <Windows.h>
#include "Handler.h"

class Console : public Handler
{
public:
	Console(kiv_os::THandle);
	~Console();

	bool read(char* buffer, size_t length, size_t& read);
	bool write(char* buffer, size_t length, size_t& written);
	int fseek(long offset, uint8_t origin);

private:
	HANDLE mStdIn;
	HANDLE mStdOut;
	HANDLE mStdError;
	bool mStdInOpen; 
	bool mStdOutOpen;
	bool mStdErrorOpen;
};

