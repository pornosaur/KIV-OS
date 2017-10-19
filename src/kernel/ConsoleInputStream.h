/*

 History
 =======

  October 20, 2011, txkoutny@kiv.zcu.cz 
    * fixed DWORD vs size_t for 64bit

  June 23, 2011, txkoutny@kiv.zcu.cz
	+ Created

*/

#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

class CConsoleInputStream{
private:
#ifdef _WIN32
	HANDLE mStdIn;
	bool mRedirectedStdIn;
	bool mStdInOpen;
#endif


public:
	CConsoleInputStream();
	~CConsoleInputStream();

	int read();

	int read(char* b);

	int read(char* b, size_t offset, size_t length);	
};

#ifdef jbcc_use_rc
	typedef std::shared_ptr<CConsoleInputStream> CConsoleInputStream_heap;
#else
    typedef CConsoleInputStream *CConsoleInputStream_heap;
#endif

inline CConsoleInputStream_heap halloc_CConsoleInputStream() {
	CConsoleInputStream_heap tmp( new CConsoleInputStream() );
	return tmp;
}