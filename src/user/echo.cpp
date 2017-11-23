#include "echo.h"
#include "rtl.h"

size_t __stdcall echo(const kiv_os::TRegisters regs) {
	const char* params = reinterpret_cast<char*> (regs.rdi.r);

	size_t writen;

	kiv_os_rtl::Write_File(kiv_os::stdOutput, params, strlen(params), writen);

	return writen;
}
