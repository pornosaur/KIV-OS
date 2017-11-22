#include "sort.h"

#include "rtl.h"


size_t __stdcall sort(const kiv_os::TRegisters &regs) {
	
	std::list<std::string> mylist;
	std::list<std::string>::iterator it;
	size_t read = 0;

	char *input = (char *)malloc(BUFFER_SIZE * sizeof(char));
	if (!input) {
		kiv_os_rtl::print_error("Out of memory.");
		return 0;
	}

	bool res;
	do {
		res = kiv_os_rtl::Read_File(kiv_os::stdInput, input, BUFFER_SIZE, read);
		if (!res || read == 0) break;

		std::string tmp = std::string(input, read);

		mylist.push_back(tmp);
		
	} while (res && read > 0);

	free(input);
	
	mylist.sort(compare_nocase);

	size_t writen = 0;
	for (it = mylist.begin(); it != mylist.end(); ++it)
	{
		kiv_os_rtl::Write_File(kiv_os::stdOutput, (*it).c_str(), (*it).size(), writen);
		if (!res || writen == 0) break;
	}

	return 0;
}

bool __stdcall compare_nocase(const std::string& first, const std::string& second)
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
