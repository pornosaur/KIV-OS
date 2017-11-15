#include "wc.h"
#include "rtl.h"
#include "string.h"

#include <regex>
#include <iostream>


size_t __stdcall wc(const kiv_os::TRegisters &regs) {
	//char* params = reinterpret_cast<char*> (regs.rdi.r);
	char *params = "Ahoj\n jak se mas?\nJa\ndobre, haha";			//TODO get params from regs
	std::string str(params);

	std::smatch match;
	std::regex reg("([^\\s]*)(\\s*)");

	size_t lines = 0, words = 0, characters = 0;
	bool gap = false;
	while (!str.empty() && std::regex_search(str, match, reg)) {
		if (!match[2].str().empty()) {
			std::string tmp = match[2].str();
			lines += std::count(tmp.begin(), tmp.end(), '\n');
		}

		words++;
		characters += match[1].str().length() + match[2].str().length();
		str = match.suffix();
	}

	str = std::to_string(lines) + " " + std::to_string(words) + " " + std::to_string(characters) + "\n";	//TODO FILE NAME
	params = kiv_os_str::copy_string(str);

	size_t written;
	kiv_os_rtl::Write_File(2, params, str.size(), written);		//TODO SET OUTPUT FROM REGS

	free(params);
	params = NULL;

	return 0;
}