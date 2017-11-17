#include "commands.h"
#include "rtl.h"

#include <cstring>

size_t __stdcall md(const kiv_os::TRegisters &regs) {
	return 0;
}

size_t __stdcall rd(const kiv_os::TRegisters &regs) {
	return 0;
}

size_t __stdcall echo(const kiv_os::TRegisters &regs) {
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	const char* params = tsi->arg;

	size_t writen;	

	kiv_os_rtl::Write_File(kiv_os::stdOutput, params, strlen(params), writen);
	kiv_os_rtl::Close_File(kiv_os::stdOutput);
	return writen;
}


size_t __stdcall sort(const kiv_os::TRegisters &regs) {
	return 0;
}


size_t __stdcall ps(const kiv_os::TRegisters &regs) {
	return 0;
}

size_t __stdcall shutdown(const kiv_os::TRegisters &regs) {
	return 0;
}
