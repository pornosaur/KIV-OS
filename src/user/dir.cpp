#include "dir.h"
#include <vector>

#include "rtl.h"

size_t __stdcall dir(const kiv_os::TRegisters regs)
{
	std::smatch match;
	std::string str(reinterpret_cast<char*> (regs.rdi.r));

	bool recursively = false;

	check_params(str, recursively);
	str.erase(0, str.find_first_not_of(ERASE_CHARS));

	if (str.empty()) {
		if (!get_current_dir(str)) {
			return 0;
		}
	}

	if (!str.find("/?")) {
		dir_print_help();
		return 0;
	}

	while (!str.empty() && std::regex_search(str, match, reg_dir)) {
		std::string tmp = match[0].str();
		str = match.suffix();

		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(ERASE_CHARS) + 1);
			tmp.erase(0, tmp.find_first_not_of(ERASE_CHARS));

			if (!print_directory(tmp, regs, recursively)) {
				kiv_os_rtl::print_error();
				return 0;
			}
		}
	}
	return 0;
}


bool print_directory(std::string path, const kiv_os::TRegisters &regs, bool &recursively)
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
	std::vector<kiv_os::TDir_Entry> entries;
	kiv_os::TDir_Entry *entry;
	size_t read = 0;
	bool res;

	print_name(path);
	path.append(DELIMETER);

	do {
		res = kiv_os_rtl::Read_File(handle, input, input_size, read);
		if (!res || read != input_size) break;

		entry = reinterpret_cast<kiv_os::TDir_Entry *>(input);
		if (entry) {
			res = print_entry(entry);

			if (res && recursively && (entry->file_attributes & kiv_os::faDirectory) == kiv_os::faDirectory) {
				entries.push_back(*entry);
			}
		}

	} while (res);

	free(input);
	if (!kiv_os_rtl::Close_File(handle)) {
		return false;
	}

	if (!res) {
		return false;
	}

	for (kiv_os::TDir_Entry en : entries) {
		std::string tmp(path);
		std::string name(en.file_name, en.file_name + sizeof en.file_name / sizeof en.file_name[0]);
		name.erase(name.find_last_not_of('\0') + 1);
		if (!print_directory(tmp.append(name), regs, recursively)) {
			return false;
		}
	}

	return true;
}


bool print_entry(kiv_os::TDir_Entry *entry)
{
	size_t writen;

	std::string name(entry->file_name, entry->file_name + sizeof entry->file_name / sizeof entry->file_name[0]);
	name.erase(name.find_last_not_of('\0') + 1);
	if ((entry->file_attributes & kiv_os::faDirectory) == kiv_os::faDirectory) {
		name.append("\t\t<DIR>");
	}
	name.append("\n");

	return kiv_os_rtl::Write_File(kiv_os::stdOutput, name.c_str(), name.length(), writen);
}


bool print_name(std::string name)
{
	size_t writen;
	return kiv_os_rtl::Write_File(kiv_os::stdOutput, ("\n" + name + "\n\n").c_str(), name.length() + 3, writen);
}


bool get_current_dir(std::string &path)
{
	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return false;
	}

	if (!kiv_os_rtl::Get_Current_Direcotry(buffer, BUFFER_SIZE, writen))
	{
		free(buffer);
		kiv_os_rtl::print_error();
		return false;;
	}

	path = std::string(buffer);
	free(buffer);
	return true;
}


void check_params(std::string &parameters, bool &recursively)
{
	std::smatch match;

	bool was_match = false;

	while (!parameters.empty() && std::regex_search(parameters, match, reg_dir)) {

		if (!match[1].str().empty()) {
			recursively = true;
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


void dir_print_help()
{
	size_t writen;

	std::string text("Displays a list of files and subdirectories in a directory.\n\nDIR [drive:][path][filename] [/S]\n\n\t[drive:][path][filename]\n\t\tSpecifies drive, directory, and/or files to list.\n\n\t/S\tDisplays files in specified directory and all subdirectories.\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, text.c_str(), text.size(), writen);
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}