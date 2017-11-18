#include "type.h"
#include <cassert>

size_t __stdcall type(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;
	
	bool console = false;
	std::smatch match;
	std::string str(params);
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
				return 0; // TODO ERROR
			}

			write_file_name(counter, kiv_os::stdOutput, tmp);
			read_and_write(handle, kiv_os::stdOutput);

			if (!console) kiv_os_rtl::Close_File(handle); // TODO check return code of Close_File?
		}
		counter++;
	}

	free(params);
	params = NULL;		//TODO clear this pointer?

	return 0; // TODO what return
}

/**
 * write data all data from in to out
 */
void read_and_write(kiv_os::THandle &in, kiv_os::THandle out) {

	char *input = (char *)malloc(buffer_size * sizeof(char));
	size_t read = 0, writen = 0;
	bool res = true;

	do {
		res = kiv_os_rtl::Read_File(in, input, buffer_size, read);
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

/**
 * if counter != 0 write name to stdout_t
 */
void write_file_name(int &counter, kiv_os::THandle stdout_t, std::string &name)
{
	if (counter) {
		size_t writen = 0;

		bool res = kiv_os_rtl::Write_File(
			stdout_t, 
			std::string("\n").append(name.append("\n\n\n")).c_str(),
			name.length() + 4,
			writen);

		if (!res || writen == 0) return; // TODO error
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