#include "md.h"

size_t __stdcall md(const kiv_os::TRegisters &regs) {
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	kiv_os::THandle stdout_t = tsi->stdout_t;
	char* params = tsi->arg;

	std::smatch match;
	std::string str(params);
	kiv_os::THandle handle;

	while (!str.empty() && std::regex_search(str, match, reg_md)) {
		std::string tmp = match[0].str();
		str = match.suffix();

		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(" \n\r\t\"'") + 1);
			tmp.erase(0, tmp.find_first_not_of(" \n\r\t\"'"));


			handle = kiv_os_rtl::Create_File(tmp.c_str(), kiv_os::fmOpen_Always, kiv_os::faDirectory);
			if (!handle) {
				return 0; // TODO error
			}

			kiv_os_rtl::Close_File(handle);

		}
	}

	free(params);
	params = NULL;		//TODO clear this pointer?

	return 0; // TODO what return
}