#include "md.h"

size_t __stdcall md(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;

	std::smatch match;
	std::string str(params);
	bool empty = true;

	while (!str.empty() && std::regex_search(str, match, reg_md_multi)) {
		std::string tmp = match[0].str();
		str = match.suffix();

		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(erase_chars) + 1);
			tmp.erase(0, tmp.find_first_not_of(erase_chars));

			empty = false;
			create_directories(tmp);
		}
	}

	if (empty) {
		kiv_os_rtl::print_error("The syntax of the command is incorrect.");
	}

	return 0; // TODO what return
}

void create_directories(std::string &path)
{
	bool created = false;
	bool res;
	
	kiv_os::THandle handle;
	std::smatch match;
	std::string tmp_path;

	while (!path.empty() && std::regex_search(path, match, reg_md_recur)) {
		std::string tmp(match[0].str());
		path = match.suffix();

		if (!tmp.empty()) {
			tmp_path.append(tmp);
			
			// open dir if exists
			handle = kiv_os_rtl::Create_File(tmp_path.c_str(), kiv_os::fmOpen_Always, kiv_os::faDirectory);
			if (handle) {
				res = kiv_os_rtl::Close_File(handle);
				if (!res) {
					kiv_os_rtl::print_error();
					return;
				}
				tmp_path.append(delimeter);
				continue;
			}

			// create dir
			handle = kiv_os_rtl::Create_File(tmp_path.c_str(), 0, kiv_os::faDirectory);
			if (!handle) {
				kiv_os_rtl::print_error();
				return;
			}

			res = kiv_os_rtl::Close_File(handle);
			if (!res) {
				kiv_os_rtl::print_error();
				return;
			}
			tmp_path.append(delimeter);
			created = true;
		}
	}

	if (!created) {
		kiv_os_rtl::print_error(tmp_path.append(": Directory or subdirectory already exists."));
		return;
	}
}