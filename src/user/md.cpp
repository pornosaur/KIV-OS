#include "md.h"

size_t __stdcall md(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;

	std::smatch match;
	std::string str(params);

	while (!str.empty() && std::regex_search(str, match, reg_md_multi)) {
		std::string tmp = match[0].str();
		str = match.suffix();

		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(erase_chars) + 1);
			tmp.erase(0, tmp.find_first_not_of(erase_chars));

			create_directories(tmp);
		}
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
					md_print_error();
					return;
				}
				tmp_path.append(delimeter);
				continue;
			}

			// create dir
			handle = kiv_os_rtl::Create_File(tmp_path.c_str(), 0, kiv_os::faDirectory);
			if (!handle) {
				md_print_error();
				return;
			}

			res = kiv_os_rtl::Close_File(handle);
			if (!res) {
				md_print_error();
				return;
			}
			tmp_path.append(delimeter);
			created = true;
		}
	}

	if (!created) {
		md_print_msg(tmp_path.append(": Directory or subdirectory already exists."));
		return;
	}
}

void md_print_error()
{
	switch (kiv_os_rtl::Get_Last_Error()) {
	case kiv_os::erInvalid_Handle:
		md_print_msg("Internal error. (Invalid Handle)");
		break;

	case kiv_os::erInvalid_Argument:
		md_print_msg("Invalid input arugments.");
		break;

	case kiv_os::erFile_Not_Found:
		md_print_msg("System can not find path.");
		break;

	case kiv_os::erDir_Not_Empty:
		md_print_msg("Directory is not empty.");
		break;

	case kiv_os::erNo_Left_Space:
		md_print_msg("Out of disk space.");
		break;

	case kiv_os::erPermission_Denied:
		md_print_msg("Operation is not permitted.");
		break;

	case kiv_os::erOut_Of_Memory:
		md_print_msg("Out of memory.");
		break;

	case kiv_os::erIO:
		md_print_msg("Disk error.");
		break;
	}
}

void md_print_msg(std::string msg)
{
	size_t writen = 0;

	msg.append("\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, msg.c_str(), msg.size(), writen);
	if (!res) {
		return;
	}
}