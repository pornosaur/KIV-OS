#include "echo.h"
#include "rtl.h"

size_t __stdcall echo(const kiv_os::TRegisters &regs) {
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	const char* params = tsi->arg;

	size_t writen;

	kiv_os_rtl::Write_File(kiv_os::stdOutput, params, strlen(params), writen);

	return writen;
}
