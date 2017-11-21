#include "type.h"
#include <cassert>


size_t __stdcall type(const kiv_os::TRegisters &regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char* params = tsi->arg;

	bool console = false;
	std::smatch match;
	std::string str(params);

	str.erase(0, str.find_first_not_of(erase_chars)); // remove redunadat chars from start of string

	if (str.empty()) {
		kiv_os_rtl::print_error("The syntax of the command is incorrect.");
		return 0;
	}

	if (!str.find("/?")) {
		type_print_help();
		return 0;
	}

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
				kiv_os_rtl::print_error();
				return 0;
			}

			write_file_name(counter, tmp);
			read_and_write(handle);

			if (!console) {
				if (!kiv_os_rtl::Close_File(handle)) {
					kiv_os_rtl::print_error();
					return 0;
				}
			}
		}
		counter++;
	}

	return 0;
}


void read_and_write(kiv_os::THandle &in)
{
	char *input = (char *)malloc(buffer_size * sizeof(char));
	if (!input) {
		kiv_os_rtl::print_error("Out of memory.");
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
		kiv_os_rtl::print_error();
		return;
	}

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, "\n", 1, writen);
	if (!res || writen == 0) {
		kiv_os_rtl::print_error();
		return;
	}
}


void write_file_name(int &print, std::string &name)
{
	if (print) {
		size_t writen = 0;

		bool res = kiv_os_rtl::Write_File(
			kiv_os::stdOutput,
			std::string("\n").append(name.append("\n\n\n")).c_str(),
			name.length() + 4,
			writen);

		if (!res || writen == 0) {
			kiv_os_rtl::print_error();
			return;
		}
	}
}


bool is_string_name_lower(std::string name, std::string string)
{
	kiv_os_str::string_to_lower(name);
	return !name.compare(string);
}


void type_print_help()
{
	size_t writen;

	std::string text("Displays the contents of a text file or files.\n\nTYPE [drive:][path]filename\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, text.c_str(), text.size(), writen);
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}