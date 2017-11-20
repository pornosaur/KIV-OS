#include "md.h"

size_t __stdcall md(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;

	std::smatch match;
	std::string str(params);
	bool empty = true;

	if (!str.find("/?")) {
		md_print_help();
		return 0;
	}

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

void md_print_help()
{
	size_t writen;

	std::string text("Creates a directory.\n\nMD [drive:]path\n\nMD creates any intermediate directories in the path, if needed.\nFor example, assume \a does not exist then:\n\n\tmd \\a\\b\\c\\d\n\nis the same as:\n\n\tmd \\a\n\tcd \\a\n\tmd b\n\tcd b\n\tmd c\n\tcd c\n\tmd d\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, text.c_str(), text.size(), writen);
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}