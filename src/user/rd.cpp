#include "rd.h"

size_t __stdcall rd(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;

	std::smatch match;
	std::string str(params);

	bool quiet = false;
	bool recursively = false;

	check_params(str, recursively, quiet);
	str.erase(0, str.find_first_not_of(erase_chars));

	if (str.empty()) {
		kiv_os_rtl::print_error("The syntax of the command is incorrect.");
		return 0;
	}

	while (!str.empty() && std::regex_search(str, match, reg_rd)) {
		std::string tmp = match[0].str();
		str = match.suffix();

		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(erase_chars) + 1);
			tmp.erase(0, tmp.find_first_not_of(erase_chars));

			// Check if folder exists and if it is a directory
			kiv_os::THandle handle = kiv_os_rtl::Create_File(tmp.c_str(), kiv_os::fmOpen_Always, kiv_os::faDirectory);
			if (!handle) {
				kiv_os_rtl::print_error();
				return 0;
			}

			bool res = kiv_os_rtl::Close_File(handle);
			if (!res) {
				kiv_os_rtl::print_error();
				return 0;
			}

			if (recursively) {
				if (quiet || ask_for_deletion(tmp))
				{
					res = remove_recursively(tmp, regs);
					if (!res) {
						kiv_os_rtl::print_error();
						return 0;
					}
				}
			}
			else {
				res = kiv_os_rtl::Remove_File(tmp.c_str());
				if (!res) {
					kiv_os_rtl::print_error();
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
		kiv_os_rtl::Close_File(handle);
		kiv_os_rtl::print_error("Out of memory.");
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
			std::string name(entry->file_name, entry->file_name + sizeof entry->file_name / sizeof entry->file_name[0]);
			res = remove_recursively(tmp.append(name), regs);
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
		kiv_os_rtl::print_error("Out of memory.");
		return false;
	}

	size_t writen = 0, read = 0;
	bool res = true;

	while (res) {
		res = kiv_os_rtl::Write_File(kiv_os::stdOutput, path.c_str(), path.size(), writen);
		if (!res) {
			kiv_os_rtl::print_error();
			free(input);
			return false;
		}
		res = kiv_os_rtl::Write_File(kiv_os::stdOutput, ", Are you sure (y/n)? ", 22, writen);
		if (!res) {
			kiv_os_rtl::print_error();
			free(input);
			return false;
		}

		res = kiv_os_rtl::Read_File(kiv_os::stdInput, input, buffer_size, read);
		if (!res) {
			kiv_os_rtl::print_error();
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