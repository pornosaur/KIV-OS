#include "type.h"

size_t __stdcall type(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	kiv_os::THandle stdin_t = tsi->stdin_t;
	kiv_os::THandle stdout_t = tsi->stdout_t;
	char* params = tsi->arg;
	
	bool console = false;
	std::smatch match;
	std::string str(params);
	kiv_os::THandle handle;

	while (!str.empty() && std::regex_search(str, match, reg_type, std::regex_constants::match_not_null)) {
		std::string tmp = match[0].str();
		
		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(" \n\r\t\"'") + 1);
			tmp.erase(0, tmp.find_first_not_of(" \n\r\t\"'"));
			
			std::string con = tmp;
			kiv_os_str::string_to_lower(con);
			console = !con.compare("con");

			handle = console ? stdin_t : kiv_os_rtl::Create_File(tmp.c_str(), kiv_os::fmOpen_Always);
			// TODO check error

			read_and_write(handle, stdout_t);
			if(!console) kiv_os_rtl::Close_File(handle); // TODO only if handle is file
		}
		str = match.suffix();
	}

	free(params);
	params = NULL;		//TODO clear this pointer?

	return 0; // TODO what return
}

void read_and_write(kiv_os::THandle &in, kiv_os::THandle &out) {

	char *input = (char *)malloc(1024 * sizeof(char));
	size_t read = 0, writen = 0;
	bool res = true;

	do {
		res = kiv_os_rtl::Read_File(in, input, 1024, read);
		if (!res || read == 0) break;

		res = kiv_os_rtl::Write_File(out, input, read, writen);
	} while (res && writen > 0);

	free(input);
	if (!res) {
		return; // TODO error;
	}

	res = kiv_os_rtl::Write_File(out, "\n", 1, writen);
	if (!res || writen == 0) return; // TODO error

}