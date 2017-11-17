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
	kiv_os::THandle stdout_t = tsi->stdout_t;
	size_t writen;	

	kiv_os_rtl::Write_File(stdout_t, params, strlen(params), writen);
	
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
