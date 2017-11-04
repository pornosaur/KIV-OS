#pragma once

#include "..\api\api.h"
#include <Windows.h>
#include "IHandleObject.h"
class Console : public IHandleObject
{


public:
	Console(kiv_os::THandle);
	~Console();

	int read(char* b, size_t offset, size_t length);
	int write(char* buffer, size_t offset, size_t length);
private:
	HANDLE mStdIn;
	HANDLE mStdOut;
	HANDLE mStdError;
	bool mStdInOpen; 
	bool mStdOutOpen;
	bool mStdErrorOpen;
};

