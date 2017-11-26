#include "sort.h"

#include "rtl.h"


size_t __stdcall sort(const kiv_os::TRegisters regs)
{
	std::string str(reinterpret_cast<char*> (regs.rdi.r));
	str.erase(str.find_last_not_of(ERASE_CHARS) + 1);
	str.erase(0, str.find_first_not_of(ERASE_CHARS));

	// read buffer initialization
	char *input = (char *)malloc(BUFFER_SIZE * sizeof(char));
	if (!input) {
		kiv_os_rtl::print_error("Out of memory.");
		return 0;
	}

	kiv_os::THandle handle = str.empty() ? kiv_os::stdInput : kiv_os_rtl::Create_File(str.c_str(), kiv_os::fmOpen_Always, 0);
	if (!handle) {
		kiv_os_rtl::print_error();
		return 0;
	}

	bool res = false;
	size_t read = 0;
	std::smatch match;
	std::list<std::string> mylist;
	std::list<std::string>::iterator it;
	std::string tmp("");
	// Read data from handle and save to list
	do {
		res = kiv_os_rtl::Read_File(handle, input, BUFFER_SIZE, read);
		if (!res || read == 0) break;

		tmp.append(input, read);

		while (!tmp.empty() && std::regex_search(tmp, match, reg_sort)) {
			if (!match[0].str().empty()) {
				mylist.push_back(match[0].str());
			}
			tmp = match.suffix();
		}
	} while (res && read > 0);
	free(input);

	if (!res) { // if result in previous loop was false
		kiv_os_rtl::print_error();
		return 0;
	}
	
	if (!tmp.empty()) {
		mylist.push_back(tmp);
	}

	mylist.sort(compare_nocase); // sort list

	// Write to stdOutput
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
