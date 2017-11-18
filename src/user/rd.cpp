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

	while (!str.empty() && std::regex_search(str, match, reg_rd)) {
		std::string tmp = match[0].str();
		str = match.suffix();

		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(erase_chars) + 1);
			tmp.erase(0, tmp.find_first_not_of(erase_chars));

			// Check if folder exists and if it is a directory
			kiv_os::THandle handle = kiv_os_rtl::Create_File(tmp.c_str(), kiv_os::fmOpen_Always, kiv_os::faDirectory);
			if (!handle) {
				return 0; // TODO error Nazev adresare je neplatny
			}

			kiv_os_rtl::Close_File(handle); // TODO check return code of Close_File?

			if (recursively) {
				if (!quiet) {
					// TODO ask for deletion
				}
				remove_recursively(tmp, regs);
			}
			else {
				bool res = kiv_os_rtl::Remove_File(tmp.c_str());
				if (!res) {
					return 0; // TODO error
				}
			}
		}
	}

	free(params);
	params = NULL;		//TODO clear this pointer?

	return 0; // TODO what return
}

void remove_recursively(std::string &path, const kiv_os::TRegisters &regs) {
	
	bool res = kiv_os_rtl::Remove_File(path.c_str());
	if (res) {
		return; // succesly removed
	}

	if (kiv_os_rtl::Get_Last_Error() != kiv_os::erDir_Not_Empty) {
		return; // TODO error
	}

	remove_subfiles(path, regs);

	res = kiv_os_rtl::Remove_File(path.c_str());
	if (!res) {
		return; // TODO error
	}
}

void remove_subfiles(std::string path, const kiv_os::TRegisters &regs)
{
	kiv_os::THandle handle = kiv_os_rtl::Create_File(path.c_str(), kiv_os::fmOpen_Always, kiv_os::faDirectory);
	if (!handle) {
		return; // TODO error
	}

	size_t input_size = sizeof(kiv_os::TDir_Entry);
	char *input = (char *)malloc(input_size);
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
			remove_recursively(tmp.append(entry->file_name), regs); // TODO handle error
		}

	} while (res && read > 0);

	free(input);

	if (!res) {
		return; // TODO error
	}
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