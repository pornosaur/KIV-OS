#pragma once

#include "..\api\api.h"
#include <Windows.h>
#include "Handler.h"

class Console : public Handler
{
public:
	Console(kiv_os::THandle);
	~Console();

	uint16_t read(char* buffer, size_t length, size_t& read);
	uint16_t write(char* buffer, size_t length, size_t& written);
	uint16_t fseek(long offset, uint8_t origin, uint8_t set_size);

private:
	HANDLE mStdIn;
	HANDLE mStdOut;
	HANDLE mStdError;
	bool mStdInOpen; 
	bool mStdOutOpen;
	bool mStdErrorOpen;
};

