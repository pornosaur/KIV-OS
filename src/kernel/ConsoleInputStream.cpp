#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <limits.h>
#else
#include <string.h>
#endif

#include "ConsoleInputStream.h"



CConsoleInputStream::CConsoleInputStream() {

#ifdef _WIN32
	_setmode( _fileno( stdin ), _O_BINARY );
	mStdIn = GetStdHandle(STD_INPUT_HANDLE);
	mRedirectedStdIn = mStdIn != (HANDLE) 3;
	mStdInOpen = true;
#endif
}

CConsoleInputStream::~CConsoleInputStream() {
}

int CConsoleInputStream::read(){
	int tmp= getchar();
	return tmp != EOF ? tmp : -1;
}

int CConsoleInputStream::read(char* b) {
	//until the length(array) is implemented, read just one char

	int tmp= getchar();

	if (tmp!= EOF) {
		*b = (char) tmp;
		return 1;
	} else
		return -1;
}

int CConsoleInputStream::read(char* b, size_t offset, size_t length) {

#ifdef _WIN32

	//Note that the following is such a tricky combination
	//that the use of fread, setting length to zero with closed
	//stdin, etc. won't work correctly. So, do not change this!

	if (mStdInOpen) {

		DWORD read;
		DWORD lengthtrim = ULONG_MAX;
		if (length<(size_t) ULONG_MAX) lengthtrim = (DWORD) length;
			//size_t could be greater than DWORD, so we might have to trim

		BOOL res = ReadFile(mStdIn, &b[offset], lengthtrim, &read, NULL);

		mStdInOpen = (res) & (read>0);

		if ((mStdInOpen) & (!mRedirectedStdIn)) {
			mStdInOpen = !
					((read>2) &&			//there was something before Ctrl+Z
 				     (b[read-3] == 0x1a) & (b[read-2] == 0x0d) & (b[read-1] == 0x0a));
			if ((!mStdInOpen) & (read>2)) read -= 3;
					//delete the sequence, if it is necessary
			return read>0 ?  read : -1;
		}

		return mStdInOpen ? read : -1;
	} else
		  return -1;	//stdin is no longer open
#else
	char* res = fgets(&b[offset], length, stdin);

	return res != NULL ? strlen(&b[offset]) : -1;
#endif
}
