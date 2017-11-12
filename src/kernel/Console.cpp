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

bool Console::read(char* buffer, size_t offset, size_t length, size_t& read) {

	//Note that the following is such a tricky combination
	//that the use of fread, setting length to zero with closed
	//stdin, etc. won't work correctly. So, do not change this!

	if (mStdInOpen) {
		DWORD read_dw, lengthtrim = ULONG_MAX;

		if (length < (size_t)ULONG_MAX) lengthtrim = (DWORD)length;
		//size_t could be greater than DWORD, so we might have to trim

		BOOL res = ReadFile(mStdIn, &buffer[offset], lengthtrim, &read_dw, NULL);

		mStdInOpen = (res) & (read_dw > 0);

		if (mStdInOpen) {
			mStdInOpen = !
				((read_dw > 2) &&			//there was something before Ctrl+Z
				(buffer[read_dw - 3] == 0x1a) & (buffer[read_dw - 2] == 0x0d) & (buffer[read_dw - 1] == 0x0a));
			
			if ((!mStdInOpen) & (read_dw > 2)) {
				read_dw -= 3;
			}
			//delete the sequence, if it is necessary

			read = read_dw > 0 ? (size_t)read_dw : 0;
			return res;
		}
		
		read = mStdInOpen ? (size_t)read_dw : 0;
		mStdInOpen = true; //TODO only for testing, change it but how?
		return res;
	}
	else {
		read = 0;		//TODO only for testing, change it but how? Console close after press ctrl+z in subshell and stay closed in init shell
		mStdInOpen = true; //TODO only for testing, change it but how?
		return false;	//stdin is no longer open
	}
		
}

bool Console::write(char* buffer, size_t offset, size_t length, size_t& written) 
{
	if (mStdOutOpen || mStdError) {
		DWORD write, lengthtrim = ULONG_MAX;

		if (length < (size_t)ULONG_MAX) {
			lengthtrim = (DWORD)length;
		}

		HANDLE mStd = (mStdOutOpen) ? mStdOut : mStdError;
		BOOL res = WriteFile(mStd, buffer, lengthtrim, &write, NULL);
		written = res ? (size_t)write : 0;
		return res;
	}
	else
		return false;
}


Console::~Console()
{
}
