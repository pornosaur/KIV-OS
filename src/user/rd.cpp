#include "rd.h"

size_t __stdcall rd(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;

	std::smatch match;
	std::string str(params);

	free(params);
	params = NULL;		//TODO clear this pointer?

	bool quiet = false;
	bool recursively = false;

	check_params(str, recursively, quiet);

	while (!str.empty() && std::regex_search(str, match, reg_rd)) {
		std::string tmp = match[0].str();
		str = match.suffix();

		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(erase_chars) + 1);
			tmp.erase(0, tmp.find_first_not_of(erase_chars));

			// Check if folder exists and if it is a directory
			kiv_os::THandle handle = kiv_os_rtl::Create_File(tmp.c_str(), kiv_os::fmOpen_Always, kiv_os::faDirectory);
			if (!handle) {
				rd_print_error();
				return 0;
			}

			bool res = kiv_os_rtl::Close_File(handle);
			if (!res) {
				rd_print_error();
				return 0;
			}

			if (recursively) {
				if (!quiet && ask_for_deletion(tmp))
				{
					res = remove_recursively(tmp, regs);
					if (!res) {
						rd_print_error();
						return 0;
					}
				}
			}
			else {
				res = kiv_os_rtl::Remove_File(tmp.c_str());
				if (!res) {
					rd_print_error();
					return 0;
				}
			}
		}
	}

	return 0; // TODO what return
}

bool remove_recursively(std::string &path, const kiv_os::TRegisters &regs) {
	
	bool res = kiv_os_rtl::Remove_File(path.c_str());
	if (res) {
		return true; // succesly removed
	}

	if (kiv_os_rtl::Get_Last_Error() != kiv_os::erDir_Not_Empty) {
		return false;
	}

	res = remove_subfiles(path, regs);
	if (!res) {
		return false;
	}

	res = kiv_os_rtl::Remove_File(path.c_str());
	if (!res) {
		return false;
	}

	return true;
}

bool remove_subfiles(std::string path, const kiv_os::TRegisters &regs)
{
	kiv_os::THandle handle = kiv_os_rtl::Create_File(path.c_str(), kiv_os::fmOpen_Always, kiv_os::faDirectory);
	if (!handle) {
		return false;
	}

	size_t input_size = sizeof(kiv_os::TDir_Entry);
	
	char *input = (char *)malloc(input_size);
	if (!input) {
		rd_print_msg("Out of memory.");
		return false;
	}

	kiv_os::TDir_Entry *entry;
	size_t read = 0;
	bool res;
	path.append(delimeter);

	do {
		res = kiv_os_rtl::Read_File(handle, input, input_size, read);
		if (!res || read != input_size) break;

		entry = reinterpret_cast<kiv_os::TDir_Entry *>(input);
		if (entry) {
			std::string tmp(path);
			res = remove_recursively(tmp.append(entry->file_name), regs);
		}

	} while (res && read > 0);

	free(input);
	if (!kiv_os_rtl::Close_File(handle)) {
		return false;
	}

	if (!res) {
		return false;
	}
	return true;
}

void check_params(std::string &parameters, bool &recursively, bool &quiet)
{
	std::smatch match;

	bool was_match = false;

	while (!parameters.empty() && std::regex_search(parameters, match, reg_rd_params)) {
		if (!match[1].str().empty()) {
			recursively = true;
			was_match = true;
		}

		if (!match[2].str().empty()) {
			quiet = true;
			was_match = true;
		}

		if (was_match) {
			parameters = match.suffix();
			was_match = false;
		}
		else {
			break;
		}
	}
}

bool ask_for_deletion(std::string &path) {

	char *input = (char *)malloc(buffer_size * sizeof(char));
	if (!input) {
		rd_print_msg("Out of memory.");
		return false;
	}

	size_t writen = 0, read = 0;
	bool res = true;

	while (res) {
		res = kiv_os_rtl::Write_File(kiv_os::stdOutput, path.c_str(), path.size(), writen);
		if (!res) {
			rd_print_error();
			free(input);
			return false;
		}
		res = kiv_os_rtl::Write_File(kiv_os::stdOutput, ", Are you sure (y/n)? ", 22, writen);
		if (!res) {
			rd_print_error();
			free(input);
			return false;
		}

		res = kiv_os_rtl::Read_File(kiv_os::stdInput, input, buffer_size, read);
		if (!res) {
			rd_print_error();
			free(input);
			return false;
		}

		if (input[0] == 'y' || input[0] == 'Y') {
			free(input);
			return true;
		}
		if (input[0] == 'n' || input[0] == 'N') {
			free(input);
			return false;
		}
	}

	free(input);
	return false;
}

void rd_print_error()
{
	switch (kiv_os_rtl::Get_Last_Error()) {
	case kiv_os::erInvalid_Handle:
		rd_print_msg("Internal error. (Invalid Handle)");
		break;

	case kiv_os::erInvalid_Argument:
		rd_print_msg("Invalid input arugments.");
		break;

	case kiv_os::erFile_Not_Found:
		rd_print_msg("System can not find path.");
		break;

	case kiv_os::erDir_Not_Empty:
		rd_print_msg("Directory is not empty.");
		break;

	case kiv_os::erNo_Left_Space:
		rd_print_msg("Out of disk space.");
		break;

	case kiv_os::erPermission_Denied:
		rd_print_msg("Operation is not permitted.");
		break;

	case kiv_os::erOut_Of_Memory:
		rd_print_msg("Out of memory.");
		break;

	case kiv_os::erIO:
		rd_print_msg("Disk error.");
		break;
	}
}

void rd_print_msg(std::string msg)
{
	size_t writen = 0;

	msg.append("\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, msg.c_str(), msg.size(), writen);
	if (!res) {
		return;
	}
}