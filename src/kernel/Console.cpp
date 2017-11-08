#include "Console.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <limits.h>


Console::Console(kiv_os::THandle handle)
{
	
	if (handle == kiv_os::stdInput) {
		_setmode(_fileno(stdin), _O_BINARY);
		mStdIn = GetStdHandle(STD_INPUT_HANDLE);
		mStdInOpen = true;
	}
	if (handle == kiv_os::stdOutput) {
		_setmode(_fileno(stdout), _O_BINARY);
		mStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		mStdOutOpen = true;
	}
	if (handle == kiv_os::stdError) {
		_setmode(_fileno(stderr), _O_BINARY);
		mStdError = GetStdHandle(STD_ERROR_HANDLE);
		mStdErrorOpen = true;
	}

}

size_t Console::read(char* buffer, size_t offset, size_t length) {

	//Note that the following is such a tricky combination
	//that the use of fread, setting length to zero with closed
	//stdin, etc. won't work correctly. So, do not change this!

	if (mStdInOpen) {
		DWORD lengthtrim = ULONG_MAX;
		DWORD read;
		if (length < (size_t)ULONG_MAX) lengthtrim = (DWORD)length;
		//size_t could be greater than DWORD, so we might have to trim

		BOOL res = ReadFile(mStdIn, &buffer[offset], lengthtrim, &read, NULL);

		mStdInOpen = (res) & (read>0);

		if (mStdInOpen) {
			mStdInOpen = !
				((read>2) &&			//there was something before Ctrl+Z
				(buffer[read - 3] == 0x1a) & (buffer[read - 2] == 0x0d) & (buffer[read - 1] == 0x0a));
			if ((!mStdInOpen) & (read > 2)) read -= 3;
			//delete the sequence, if it is necessary
			return read > 0 ? (size_t)read : -1;
		}

		return mStdInOpen ? (size_t)read : -1;
	}
	else
		return -1;	//stdin is no longer open
}

size_t Console::write(char* buffer, size_t offset, size_t length) {
	
	if (mStdOutOpen || mStdError) {
		DWORD write;
		DWORD lengthtrim = ULONG_MAX;
		if (length < (size_t)ULONG_MAX) lengthtrim = (DWORD)length;
		HANDLE mStd = (mStdOutOpen) ? mStdOut : mStdError;
		BOOL res = WriteFile(mStd, buffer, lengthtrim, &write, NULL);
		return res ? write : -1;
	}else
		return -1;
}


Console::~Console()
{
}
