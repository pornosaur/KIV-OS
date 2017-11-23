#include "sort.h"

#include "rtl.h"


size_t __stdcall sort(const kiv_os::TRegisters regs)
{
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);

	std::string str(tsi->arg);
	str.erase(str.find_last_not_of(ERASE_CHARS) + 1);
	str.erase(0, str.find_first_not_of(ERASE_CHARS));

	std::list<std::string> mylist;
	std::list<std::string>::iterator it;

	// read buffer initialization
	char *input = (char *)malloc(BUFFER_SIZE * sizeof(char));
	if (!input) {
		kiv_os_rtl::print_error("Out of memory.");
		return 0;
	}

	kiv_os::THandle handle = str.empty() ? kiv_os::stdInput : kiv_os_rtl::Create_File(str.c_str(), kiv_os::fmOpen_Always, 0);

	bool res = false;
	std::smatch match;
	size_t read = 0;

	do {
		res = kiv_os_rtl::Read_File(handle, input, BUFFER_SIZE, read);
		if (!res || read == 0) break;

		std::string tmp(input, read);

		while (!tmp.empty() && std::regex_search(tmp, match, reg_sort)) {
			if (!match[0].str().empty()) {
				mylist.push_back(match[0].str().append("\n"));
			}
			tmp = match.suffix();
		}
		
	} while (res && read > 0);
	free(input);

	if (!res) {
		kiv_os_rtl::print_error();
		return 0;
	}
	
	mylist.sort(compare_nocase);

	size_t writen = 0;
	for (it = mylist.begin(); it != mylist.end(); ++it)
	{
		res = kiv_os_rtl::Write_File(kiv_os::stdOutput, (*it).c_str(), (*it).size(), writen);
		if (!res || writen == 0) break;
	}

	if (!res) {
		kiv_os_rtl::print_error();
		return 0;
	}

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, "\n\n", 2, writen);
	if (!res) {
		kiv_os_rtl::print_error();
		return 0;
	}

	return 0;
}

bool compare_nocase(const std::string& first, const std::string& second)
{
	size_t i = 0;
	while ((i < first.length()) && (i<second.length()))
	{
		if (tolower(first[i]) < tolower(second[i])) {
			return true;
		}
		else if (tolower(first[i]) > tolower(second[i])) {
			return false;
		}
		i++;
	}
	return (first.length() < second.length());
}
