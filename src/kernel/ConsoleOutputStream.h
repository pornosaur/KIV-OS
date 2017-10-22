
#pragma once

#include <Windows.h>

class ConsoleOutputStream {
private:
	HANDLE mStdOut;
	bool mStdOutOpen;



public:
	ConsoleOutputStream(bool is_std_err);
	~ConsoleOutputStream();

	int write(char c);

	int write(char* buffer, size_t length);
};


