#include "commands.h"
#include "rtl.h"

#include <cstring>

size_t __stdcall type(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;
	kiv_os::THandle stdout_t = tsi->stdout_t;

	kiv_os::THandle handle = kiv_os_rtl::Create_File(params, kiv_os::fmOpen_Always);
	handle = tsi->stdin_t;
	// TODO check error
	free(params);
	params = NULL;		//TODO clear this pointer?

	char *input = (char *)malloc(1024 * sizeof(char));
	size_t read = 0, writen = 0;
	bool res = true;

	do {
		res = kiv_os_rtl::Read_File(handle, input, 1024, read);
		if (!res || read == 0) break;

		res = kiv_os_rtl::Write_File(stdout_t, input, read, writen);
	} while (res && writen > 0);
	free(input);

	res = kiv_os_rtl::Write_File(stdout_t, "\n", 1, writen);
	if (!res || writen == 0) return 0; // TODO error

	return 0; // TODO what return
}

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
