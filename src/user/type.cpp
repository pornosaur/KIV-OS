#include "type.h"
#include <cassert>

size_t __stdcall type(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;
	
	bool console = false;
	std::smatch match;
	std::string str(params);

	free(params);
	params = NULL;		//TODO clear this pointer?

	kiv_os::THandle handle;
	int counter = 0;

	while (!str.empty() && std::regex_search(str, match, reg_type)) {
		std::string tmp = match[0].str();
		str = match.suffix();
		
		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(erase_chars) + 1);
			tmp.erase(0, tmp.find_first_not_of(erase_chars));

			if (is_string_name_lower(tmp, "nul")) {
				counter++;
				continue;
			}

			console = is_string_name_lower(tmp, "con");
			handle = console ? kiv_os::stdInput : kiv_os_rtl::Create_File(tmp.c_str(), kiv_os::fmOpen_Always, 0);
			if (!handle) {
				type_print_error();
				return 0;
			}

			write_file_name(counter, tmp);
			read_and_write(handle);

			if (!console) {
				bool res = kiv_os_rtl::Close_File(handle);
				if (!res) {
					type_print_error();
					return 0;
				}
			}
		}
		counter++;
	}

	return 0; // TODO what return
}

/**
 * write data all data from in to out
 */
void read_and_write(kiv_os::THandle &in) {

	char *input = (char *)malloc(buffer_size * sizeof(char));
	if (!input) {
		type_print_msg("Out of memory.");
		return;
	}

	size_t read = 0, writen = 0;
	bool res = true;

	do {
		res = kiv_os_rtl::Read_File(in, input, buffer_size, read);
		if (!res || read == 0) break;

		res = kiv_os_rtl::Write_File(kiv_os::stdOutput, input, read, writen);
	} while (res && writen > 0);

	free(input);
	if (!res) {
		type_print_error();
		return;
	}

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, "\n", 1, writen);
	if (!res || writen == 0) {
		type_print_error();
		return;
	}
}

/**
 * if counter != 0 write name to stdout_t
 */
void write_file_name(int &counter, std::string &name)
{
	if (counter) {
		size_t writen = 0;

		bool res = kiv_os_rtl::Write_File(
			kiv_os::stdOutput, 
			std::string("\n").append(name.append("\n\n\n")).c_str(),
			name.length() + 4,
			writen);

		if (!res || writen == 0) {
			type_print_error();
			return;
		}
	}
}

/**
 * return true if name.lowerCase is equals to string 
 */
bool is_string_name_lower(std::string name, std::string string)
{
	kiv_os_str::string_to_lower(name);
	return !name.compare(string);
}

void type_print_error()
{
	switch (kiv_os_rtl::Get_Last_Error()) {
	case kiv_os::erInvalid_Handle:
		type_print_msg("Internal error. (Invalid Handle)");
		break;

	case kiv_os::erInvalid_Argument:
		type_print_msg("Invalid input arugments.");
		break;

	case kiv_os::erFile_Not_Found:
		type_print_msg("System can not find path.");
		break;

	case kiv_os::erDir_Not_Empty:
		type_print_msg("Directory is not empty.");
		break;

	case kiv_os::erNo_Left_Space:
		type_print_msg("Out of disk space.");
		break;

	case kiv_os::erPermission_Denied:
		type_print_msg("Operation is not permitted.");
		break;

	case kiv_os::erOut_Of_Memory:
		type_print_msg("Out of memory.");
		break;

	case kiv_os::erIO:
		type_print_msg("Disk error.");
		break;
	}
}

void type_print_msg(std::string msg)
{
	size_t writen = 0;

	msg.append("\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, msg.c_str(), msg.size(), writen);
	if (!res) {
		return;
	}
}