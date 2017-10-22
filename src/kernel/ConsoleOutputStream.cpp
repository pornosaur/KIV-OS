#include <stdio.h>

#include <io.h>
#include <fcntl.h>
#include <limits.h>

#include "ConsoleOutputStream.h"



ConsoleOutputStream::ConsoleOutputStream(bool is_std_err) {
	if (is_std_err) {
		_setmode(_fileno(stderr), _O_BINARY);
		mStdOut = GetStdHandle(STD_ERROR_HANDLE);
	} else {
		_setmode(_fileno(stdout), _O_BINARY);
		mStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	
	mStdOutOpen = true;
}

ConsoleOutputStream::~ConsoleOutputStream() {
	CloseHandle(mStdOut);
}

int ConsoleOutputStream::write(char c){
	int tmp = putchar(c);
	
	if (tmp != EOF) {
		return 1;
	}
	else
		return -1;
}

int ConsoleOutputStream::write(char* buffer, size_t length) {
	if (mStdOutOpen) {
		DWORD write;
		DWORD lengthtrim = ULONG_MAX;
		if (length<(size_t)ULONG_MAX) lengthtrim = (DWORD)length;

		BOOL res = WriteFile(mStdOut, buffer, lengthtrim, &write, NULL);

		return res ? write : -1;
	}
	else
		return -1;	
}
